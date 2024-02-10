#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h>
#include "esp_log.h"
#include "../lib/lwrb/src/include/lwrb/lwrb.h"

// Define queue handle
QueueHandle_t data_queue;



#define BUTTON_GPIO 4        // GPIO pin for the button
#define RED_LED_GPIO 19      // GPIO pin for the red LED
#define BLUE_LED_GPIO 21     // GPIO pin for the blue LED
#define GREEN_LED_GPIO 18    // GPIO pin for the green LED

#define SHORT_PRESS_BIT BIT0 // Bit for short press event
#define LONG_PRESS_BIT BIT1  // Bit for long press event

#define BLUE_ON_BIT BIT2
#define BLUE_OFF_BIT BIT3
#define RED_ON_BIT BIT4
#define RED_OFF_BIT BIT5
#define SWITCH_BIT BIT6
#define DUMPLOG_BIT BIT7

#define SHORT_PRESS_THRESHOLD 500 // milliseconds
#define LONG_PRESS_THRESHOLD 800  // milliseconds

#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)

#define UART UART_NUM_0

static const int RX_BUF_SIZE = 1024;
int num = 0;

EventGroupHandle_t syncEventGroup;
lwrb_t buff;
uint8_t buff_data[31];

typedef enum{
    BTN_PRESS_LONG,
    BTN_PRESS_SHORT,
    BLUE_ON_CMD,
    BLUE_OFF_CMD,
    RED_ON_CMD,
    RED_OFF_CMD,
    SWITCH_CMD,
    DUMPLOG_CMD
}Events;

typedef struct {
    unsigned int timestamp;
    Events event;
}LogData;

const int LOG_SIZE = sizeof(LogData);


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
                    esp_restart();
                }
            }        
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling 100Hz
    }
}

void red_led_task(void *pvParameters) {
    bool red_state = false;

    gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RED_LED_GPIO, red_state); // Default off

    while(1) {
        EventBits_t bits = xEventGroupWaitBits(syncEventGroup, SHORT_PRESS_BIT | RED_ON_BIT | RED_OFF_BIT | SWITCH_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if ((bits & SHORT_PRESS_BIT) || (bits & SWITCH_BIT)) {
            red_state = !red_state;
            gpio_set_level(RED_LED_GPIO, red_state);
        } else if (bits & RED_ON_BIT) {
            red_state = true;
            gpio_set_level(RED_LED_GPIO, red_state);
            xEventGroupClearBits(syncEventGroup,RED_ON_BIT);
        } else if (bits & RED_OFF_BIT) {
            red_state = false;
            gpio_set_level(RED_LED_GPIO, red_state);
            xEventGroupClearBits(syncEventGroup,RED_OFF_BIT);
        }
    }
}

void blue_led_task(void *pvParameters) {
    bool blue_state = true;

    gpio_set_direction(BLUE_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLUE_LED_GPIO, blue_state); 

    while(1) {
        EventBits_t bits = xEventGroupWaitBits(syncEventGroup, SHORT_PRESS_BIT | BLUE_ON_BIT | BLUE_OFF_BIT | SWITCH_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if ((bits & SHORT_PRESS_BIT) || (bits & SWITCH_BIT)) {
            blue_state = !blue_state;
            gpio_set_level(BLUE_LED_GPIO, blue_state);
            xEventGroupClearBits(syncEventGroup, SHORT_PRESS_BIT);
        }else if (bits & BLUE_ON_BIT) {
            blue_state = true;
            gpio_set_level(BLUE_LED_GPIO, blue_state);
            xEventGroupClearBits(syncEventGroup,BLUE_ON_BIT);
        } else if (bits & BLUE_OFF_BIT) {
            blue_state = false;
            gpio_set_level(BLUE_LED_GPIO, blue_state);
            xEventGroupClearBits(syncEventGroup,BLUE_OFF_BIT);
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

void init(void) 
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void tx_task(void *arg)
{
    while (1) {
        char* received_data;
        if (xQueueReceive(data_queue, &received_data, portMAX_DELAY) == pdTRUE) {
            uart_write_bytes(UART, received_data, strlen(received_data));
            free(received_data);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 500 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            char* tx_data = (char*) malloc(rxBytes + 1);
            memcpy(tx_data, data, rxBytes + 1); 
            if (xQueueSend(data_queue, &tx_data, portMAX_DELAY) != pdPASS) {
                free(tx_data); 
            }

            if (strcmp((const char *)data,"BLUE;ON")==0){
                xEventGroupSetBits(syncEventGroup,BLUE_ON_BIT);
            }else if(strcmp((const char *)data,"BLUE;OFF")==0){
                xEventGroupSetBits(syncEventGroup,BLUE_OFF_BIT);
            }else if(strcmp((const char *)data,"RED;ON")==0){
                xEventGroupSetBits(syncEventGroup,RED_ON_BIT);
            }else if(strcmp((const char *)data,"RED;OFF")==0){
                xEventGroupSetBits(syncEventGroup,RED_OFF_BIT);
            }else if(strcmp((const char *)data,"SWITCH")==0){
                xEventGroupSetBits(syncEventGroup,SWITCH_BIT);
            }else if(strcmp((const char *)data,"DUMPLOG")==0){
                xEventGroupSetBits(syncEventGroup,DUMPLOG_BIT);
            }
        }

    }
    free(data);
}
void app_main() {
    syncEventGroup = xEventGroupCreate();
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    data_queue = xQueueCreate(10, sizeof(char*)); 

    init();
    xTaskCreate(rx_task, "uart_rx_task", 2048, NULL, 1, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 2048, NULL, 1, NULL);
    xTaskCreate(button_task, "Button Task", 2048, NULL, 2, NULL);
    xTaskCreate(red_led_task, "Red LED Task", 2048, NULL, 1, NULL);
    xTaskCreate(blue_led_task, "Blue LED Task", 2048, NULL, 1, NULL);
    xTaskCreate(green_led_task, "Green LED Task", 2048, NULL, 1, NULL);

    lwrb_init(&buff, buff_data, sizeof(buff_data));
}
