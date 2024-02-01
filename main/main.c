#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO 4        // GPIO pin for the button
#define RED_LED_GPIO 19      // GPIO pin for the red LED
#define BLUE_LED_GPIO 21     // GPIO pin for the blue LED
#define GREEN_LED_GPIO 18    // GPIO pin for the green LED

#define SHORT_PRESS_BIT BIT0 // Bit for short press event
#define LONG_PRESS_BIT BIT1  // Bit for long press event

#define SHORT_PRESS_THRESHOLD 500 // milliseconds
#define LONG_PRESS_THRESHOLD 800  // milliseconds

EventGroupHandle_t syncEventGroup;

void button_task(void *pvParameters) {
    bool button_state = false;
    TickType_t press_start_time = 0;
    while(1) {
        bool new_button_state = gpio_get_level(BUTTON_GPIO);
        if(new_button_state != button_state) {
            button_state = new_button_state;
            if(button_state) {
                press_start_time = xTaskGetTickCount();
            } else {
                TickType_t press_duration = xTaskGetTickCount() - press_start_time;
                if(press_duration < SHORT_PRESS_THRESHOLD / portTICK_PERIOD_MS) {
                    xEventGroupSetBits(syncEventGroup, SHORT_PRESS_BIT); // Set short press bit
                    ESP_LOGI("BUTTON", "SHORT PRESS");
                } else if(press_duration >= LONG_PRESS_THRESHOLD / portTICK_PERIOD_MS) {
                    xEventGroupSetBits(syncEventGroup, LONG_PRESS_BIT); // Set long press bit
                    ESP_LOGI("BUTTON", "LONG PRESS");
                }
            }        
        }
        vTaskDelay(pdMS_TO_TICKS(1)); // Polling 1kHz
    }
}

void red_led_task(void *pvParameters) {
    bool red_state = false;

    gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RED_LED_GPIO, red_state); // Default off

    while(1) {
        EventBits_t bits = xEventGroupWaitBits(syncEventGroup, SHORT_PRESS_BIT | LONG_PRESS_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if (bits & SHORT_PRESS_BIT) {
            // Toggle red LED state on short press
            red_state = !red_state;
            gpio_set_level(RED_LED_GPIO, red_state);
            ESP_LOGI("RED", "TOGGLE");
        } else if (bits & LONG_PRESS_BIT) {
            // Return red LED to default state (off) on long press
            red_state = false;
            gpio_set_level(RED_LED_GPIO, red_state);
            ESP_LOGI("RED", "OFF");
        }
    }
}

void blue_led_task(void *pvParameters) {
    bool blue_state = true;

    gpio_set_direction(BLUE_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLUE_LED_GPIO, blue_state); // Default on

    while(1) {
        EventBits_t bits = xEventGroupWaitBits(syncEventGroup, SHORT_PRESS_BIT | LONG_PRESS_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if (bits & SHORT_PRESS_BIT) {
            // Toggle blue LED state on short press
            blue_state = !blue_state;
            gpio_set_level(BLUE_LED_GPIO, blue_state);
            ESP_LOGI("BLUE", "TOGGLE");
        } else if (bits & LONG_PRESS_BIT) {
            // Return blue LED to default state (on) on long press
            blue_state = true;
            gpio_set_level(BLUE_LED_GPIO, blue_state);
            ESP_LOGI("BLUE", "ON");
        }
    }
}

void green_led_task(void *pvParameters) {
    gpio_set_direction(GREEN_LED_GPIO, GPIO_MODE_OUTPUT);

    while(1) {
        gpio_set_level(GREEN_LED_GPIO, 1); // Turn on green LED
        vTaskDelay(pdMS_TO_TICKS(150)); // Delay for 150 ms
        gpio_set_level(GREEN_LED_GPIO, 0); // Turn off green LED
        vTaskDelay(pdMS_TO_TICKS(150)); // Delay for 150 ms
    }
}

void app_main() {
    // Initialize GPIOs and eventGroup
    syncEventGroup = xEventGroupCreate();
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    // Create tasks for button, red LED, blue LED, and green LED
    xTaskCreate(button_task, "Button Task", 2048, NULL, 2, NULL);
    xTaskCreate(red_led_task, "Red LED Task", 2048, NULL, 1, NULL);
    xTaskCreate(blue_led_task, "Blue LED Task", 2048, NULL, 1, NULL);
    xTaskCreate(green_led_task, "Green LED Task", 2048, NULL, 1, NULL);
}
