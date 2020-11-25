
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
#include "json_parser.h"

static const char *TAG = "HCCLIENT";

#if 0
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
            if (!esp_http_client_is_chunked_response(evt->client)) {
                ESP_LOGD(TAG, "%.*s", evt->data_len, (char*)evt->data);
            }
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
#endif
uint8_t get_hc_status(void)
{
    ESP_LOGI(TAG, "Get static called");
#if 0
    esp_err_t err = ESP_OK;
    esp_http_client_config_t config = {
        .url = CONFIG_HCSTATUS_URL,
        .event_handler = http_event_handle,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    //esp_http_client_set_url(client, CONFIG_HC_STATUS);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    err = esp_http_client_perform(client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error retrieving Hydrocut status");
    }
    uint16_t statuscode = esp_http_client_get_status_code(client);
    if (statuscode != 200)
    {
        ESP_LOGE(TAG, "Error retrieving Hydrocut status: invalid status code: %d", statuscode);
    }
    esp_http_client_cleanup(client);
    return "closed";
#endif 
    return CONTACT_DETECTED;
}
