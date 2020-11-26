
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include <esp_event.h>
#include <esp_log.h>
#include <esp_http_client.h>

#include "homekit_states.h"
#include "hydrocut_client.h"
#include "led.h"
#include "json_parser.h"

static const char *TAG = "HCCLIENT";

#ifdef CONFIG_HC_TEST_MODE    
static const uint16_t hydrocut_client_TASK_PRIORITY = 5;
static const uint16_t hydrocut_client_TASK_STACKSIZE = 4 * 1024;
static const char *hydrocut_client_TASK_NAME = "hydrocut_client";
#endif

static uint8_t hydrocut_status = CONTACT_DETECTED;
static uint16_t hydrocut_soc = 0;
static float hydrocut_air_temp = 0.0;
static float hydrocut_ground_temp = 0.0;
#define HTTP_DATA_SIZE 2048
#define JSON_ITEM_SIZE 64
static char http_data[HTTP_DATA_SIZE] = { 0 };
static size_t http_data_count = 0;

static esp_err_t http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            ESP_LOGD(TAG, "%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            strlcat(http_data, (const char*)evt->data, HTTP_DATA_SIZE);
            http_data_count+=evt->data_len;
            http_data[http_data_count] = 0;
            ESP_LOGD(TAG, "%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void parse_status(void)
{
	char status[JSON_ITEM_SIZE]={ 0 };
	jparse_ctx_t jctx;
    memset((void*)&jctx, 0, sizeof(jparse_ctx_t));
	int ret = json_parse_start(&jctx, http_data, strnlen(http_data, HTTP_DATA_SIZE));
	if (ret == OS_SUCCESS)
    {
    	if (json_obj_get_string(&jctx, "status", status, JSON_ITEM_SIZE) == OS_SUCCESS)
        {
	    	ESP_LOGI(TAG, "hc status = %s", status);
            if (strncmp(status, "closed", JSON_ITEM_SIZE) == 0)
            {
                hydrocut_status = CONTACT_DETECTED;
                ledclose_on();
            }
            else
            {
                hydrocut_status = CONTACT_NOT_DETECTED;
                ledopen_on();
            }
        }
    }
    else
    {
		ESP_LOGE(TAG, "JSON Parser failed");
	}
    json_parse_end(&jctx);
}

static void parse_thingspeak(int fieldnum, char *result, int len)
{
    char fieldname[JSON_ITEM_SIZE] = { 0 };
    int num_elem = 0;
	jparse_ctx_t jctx;

    memset((void*)&jctx, 0, sizeof(jparse_ctx_t));
    snprintf(fieldname, JSON_ITEM_SIZE, "field%d", fieldnum);
	int ret = json_parse_start(&jctx, http_data, strnlen(http_data, HTTP_DATA_SIZE));
	if (ret == OS_SUCCESS)
    {
        if (json_obj_get_array(&jctx, "feeds", &num_elem) == OS_SUCCESS) {
            if (num_elem>1)
            {
                ESP_LOGE(TAG, "Feeds array has more than one element and they will be ignored");
            }
            ESP_LOGI(TAG ,"Feeds Array has %d elements and looking for %s", num_elem, fieldname);
            for (int i = 0; i < num_elem; i++) {
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS)
                {
                    if (json_obj_get_string(&jctx, fieldname, result, len) == OS_SUCCESS)
                    {
                        ESP_LOGI(TAG, "%s = %s\n", fieldname, result);
                    }
                    else
                    {
                        ESP_LOGE(TAG, "JSON Parsing failed on json_obj_get_float");
                    }
                }
                else
                {
                        ESP_LOGE(TAG, "JSON Parsing failed on json_arr_get_object");
                }
                json_arr_leave_object(&jctx);
                break;
            }
            json_obj_leave_array(&jctx);
        }
    }
    else
    {
		ESP_LOGE(TAG, "JSON Parser failed");
	}
    json_parse_end(&jctx);
}


static uint16_t get_rest_data(char *url)
{
    uint16_t statuscode = 500;
    ESP_LOGI(TAG, "get_rest_data called");
    esp_err_t err = ESP_OK;
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handle,
        .buffer_size = HTTP_DATA_SIZE,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    http_data_count = 0;
    memset(http_data, 0, HTTP_DATA_SIZE);
    err = esp_http_client_perform(client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error retrieving Hydrocut status");
    }
    statuscode = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    return (statuscode);
}

void update_hc_status(void)
{
    uint16_t statuscode = get_rest_data(CONFIG_HCSTATUS_URL);
    if (statuscode == 200)
    {
        ESP_LOGI(TAG, "Got status data from call successfully");
        ESP_LOGI(TAG, "http_data=%s", http_data);
        parse_status();
    }
    else
    {
        ESP_LOGE(TAG, "Error retrieving Hydrocut status: invalid status code: %d", statuscode);
    }
}

static void update_pv_status(void)
{
#ifdef CONFIG_HC_CHARGECONTROLER_CHANNEL_URL
    char battery_soc[JSON_ITEM_SIZE] = { 0 };

    uint16_t statuscode = get_rest_data(CONFIG_HC_CHARGECONTROLER_CHANNEL_URL);
    if (statuscode == 200)
    {
        ESP_LOGI(TAG, "Got charge controller data from call successfully");
        ESP_LOGI(TAG, "http_data=%s", http_data);
        parse_thingspeak(CONFIG_HC_CHARGECONTROLER_SOC_FIELD_NUM, battery_soc, JSON_ITEM_SIZE);
        hydrocut_soc = atoi(battery_soc);
    }
    else
    {
        ESP_LOGE(TAG, "Error retrieving Hydrocut status: invalid status code: %d", statuscode);
    }
#else
    ESP_LOGE(TAG, "CONFIG_HC_CHARGECONTROLER_CHANNEL_URL undefined in config");
#endif
}

void update_air_temp(void)
{
#ifdef CONFIG_HC_AIR_TEMP_CHANNEL_URL    
    char temp[JSON_ITEM_SIZE] = { 0 };
    uint16_t statuscode = get_rest_data(CONFIG_HC_AIR_TEMP_CHANNEL_URL);
    if (statuscode == 200)
    {
        ESP_LOGI(TAG, "Got charge controller data from call successfully");
        ESP_LOGI(TAG, "http_data=%s", http_data);
        parse_thingspeak(CONFIG_HC_AIR_TEMP_CHANNEL_FIELD_NUM, temp, JSON_ITEM_SIZE);
        hydrocut_air_temp = atof(temp);
    }
    else
    {
        ESP_LOGE(TAG, "Error retrieving Hydrocut status: invalid status code: %d", statuscode);
    }
#else
    ESP_LOGE(TAG, "CONFIG_HC_TEMP_CHANNEL_URL undefined in config");
#endif
}

static void update_ground_temp(void)
{
#ifdef CONFIG_HC_GROUND_TEMP_CHANNEL_URL    
    char temp[JSON_ITEM_SIZE] = { 0 };
    uint16_t statuscode = get_rest_data(CONFIG_HC_GROUND_TEMP_CHANNEL_URL);
    if (statuscode == 200)
    {
        ESP_LOGI(TAG, "Got ground temp data from call successfully");
        ESP_LOGI(TAG, "http_data=%s", http_data);
        parse_thingspeak(CONFIG_HC_GROUND_TEMP_CHANNEL_FIELD_NUM, temp, JSON_ITEM_SIZE);
        hydrocut_ground_temp = atof(temp);
    }
    else
    {
        ESP_LOGE(TAG, "Error retrieving Hydrocut status: invalid status code: %d", statuscode);
    }
#else
    ESP_LOGE(TAG, "CONFIG_HC_TEMP_CHANNEL_URL undefined in config");
#endif
}

uint8_t get_hc_status(void)
{
#ifndef CONFIG_HC_TEST_MODE    
    update_hc_status();
#endif
    return (hydrocut_status);
}

uint16_t get_hc_soc(void)
{
#ifndef CONFIG_HC_TEST_MODE    
    update_pv_status();
#endif
    return (hydrocut_soc);
}

float get_hc_ground_temp(void)
{
#ifndef CONFIG_HC_TEST_MODE    
    update_ground_temp();
#endif
    return (hydrocut_ground_temp);
}

float get_hc_air_temp(void)
{
#ifndef CONFIG_HC_TEST_MODE    
    update_air_temp();
#endif
    return (hydrocut_air_temp);
}

#ifdef CONFIG_HC_TEST_MODE    
static void hydrocut_client_thread_entry(void *p)
{
    const uint16_t delay =  (CONFIG_HC_STATUS_TIMEOUT*1000) / portTICK_PERIOD_MS;
    while (1)
    {
        update_hc_status();
        update_pv_status();
        update_air_temp();
        update_ground_temp();
        vTaskDelay(delay);
    }
}
#endif

void hydrocut_client_start(void)
{
#ifdef CONFIG_HC_TEST_MODE    
    xTaskCreate(hydrocut_client_thread_entry, hydrocut_client_TASK_NAME, hydrocut_client_TASK_STACKSIZE, NULL, hydrocut_client_TASK_PRIORITY, NULL);
#endif
}