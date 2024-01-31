#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "string.h"
#include "esp_log.h" // Include the ESP-IDF logging header
#include "freertos/event_groups.h"

#define BUTTON_GPIO 4 // GPIO pin for the button
#define RED_LED_GPIO 19 // GPIO pin for the red LED
#define BLUE_LED_GPIO 21 // GPIO pin for the blue LED
#define GREEN_LED_GPIO 18 // GPIO pin for the green LED

#define SHORT_PRESS_THRESHOLD 500 // Adjust as needed (milliseconds)
#define LONG_PRESS_THRESHOLD 800 // Adjust as needed (milliseconds)

QueueHandle_t buttonQueue;
EventGroupHandle_t syncEventGroup;

void button_task(void *pvParameters) {
    bool button_state = false;
    TickType_t press_start_time = 0;
    int value =0;
    while(1) {
        bool new_button_state = gpio_get_level(BUTTON_GPIO);

        if(new_button_state != button_state) {
            button_state = new_button_state;

            if(button_state) {
                // Button pressed
                press_start_time = xTaskGetTickCount();
            } else {
                // Button released
                TickType_t press_duration = xTaskGetTickCount() - press_start_time;
                
                if(press_duration < SHORT_PRESS_THRESHOLD / portTICK_PERIOD_MS) {
                    value = 0;
                    xQueueSend(buttonQueue, &value, 0);
                    ESP_LOGI("BUTTON", "SHORT PRESS");
                } else if(press_duration >= LONG_PRESS_THRESHOLD / portTICK_PERIOD_MS) {
                    value = 1;
                    xQueueSend(buttonQueue, &value, 0);
                    ESP_LOGI("BUTTON", "LONG PRESS");
                }
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
    }
}

void red_led_task(void *pvParameters) {
    bool red_state = false;

    gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RED_LED_GPIO, red_state); // Default off

    while(1) {
        int received;
        if (xQueueReceive(buttonQueue, &received, portMAX_DELAY)) {
            ESP_LOGI("RED", "RECEIVED");
            if (received == 0) {
                // Toggle red LED state on short press
                red_state = !red_state;
                gpio_set_level(RED_LED_GPIO, red_state);
            } else if (received == 1) {
                // Return red LED to default state (off) on long press
                red_state = false;
                gpio_set_level(RED_LED_GPIO, red_state);
            }
        }
    }
}

void blue_led_task(void *pvParameters) {
    bool blue_state = true;

    gpio_set_direction(BLUE_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLUE_LED_GPIO, blue_state); // Default on

    while(1) {
        int received_value;
        if (xQueueReceive(buttonQueue, &received_value, portMAX_DELAY)) {
            ESP_LOGI("BLUE", "RECEIVED");
            if (received_value == 0) {
                // Toggle blue LED state on short press
                blue_state = !blue_state;
                gpio_set_level(BLUE_LED_GPIO, blue_state);
            } else if (received_value == 1) {
                // Return blue LED to default state (on) on long press
                blue_state = true;
                gpio_set_level(BLUE_LED_GPIO, blue_state);
            }
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
    // Initialize GPIOs and queue
    syncEventGroup = xEventGroupCreate();
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    buttonQueue = xQueueCreate(5, sizeof(int));

    // Create tasks for button, red LED, and blue LED
    xTaskCreatePinnedToCore(button_task, "Button Task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(blue_led_task, "Blue LED Task", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(red_led_task, "Red LED Task", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(green_led_task, "Green LED Task", 2048, NULL, 1, NULL, 1);


}