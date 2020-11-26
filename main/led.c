/*
    Addressable LED support

    (C) 2020 Mark Buckaway - Apache License Version 2.0, January 2004
*/

#include "esp_log.h"
#include "driver/gpio.h"
#include "led.h"

static const char *TAG = "LED";

void led_off(void)
{
    ESP_LOGI(TAG, "LED OFF");
    gpio_set_level(CONFIG_HC_CLOSED_LED_GPIO, 0);
    gpio_set_level(CONFIG_HC_OPEN_LED_GPIO, 0);
}

void ledclose_on(void)
{
    ESP_LOGI(TAG, "LED CLOSED ON");
    gpio_set_level(CONFIG_HC_CLOSED_LED_GPIO, 1);
    gpio_set_level(CONFIG_HC_OPEN_LED_GPIO, 0);
}

void ledopen_on(void)
{
    ESP_LOGI(TAG, "LED OPEN ON");
    gpio_set_level(CONFIG_HC_CLOSED_LED_GPIO, 0);
    gpio_set_level(CONFIG_HC_OPEN_LED_GPIO, 1);
}

void led_both(void)
{
    ESP_LOGI(TAG, "LED BOTH ON");
    gpio_set_level(CONFIG_HC_CLOSED_LED_GPIO, 1);
    gpio_set_level(CONFIG_HC_OPEN_LED_GPIO, 1);
}

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<CONFIG_HC_CLOSED_LED_GPIO) | (1ULL<<CONFIG_HC_OPEN_LED_GPIO))

void configure_led(void)
{
    ESP_LOGI(TAG, "Configuring LEDS on GPIO Closed %d and Open %d", CONFIG_HC_CLOSED_LED_GPIO, CONFIG_HC_OPEN_LED_GPIO);
    gpio_config_t io_conf_leds = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf_leds);

}
