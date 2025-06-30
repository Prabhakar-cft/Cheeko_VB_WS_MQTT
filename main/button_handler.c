#include "button_handler.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_session.h"
#include "websocket_client.h"
#include "session_manager.h"
#include "mqtt_handler.h"
#include "led.h"

#define TAG "BUTTON_HANDLER"

void button_task(void *arg) {
    bool last_button_state = false; // Button not pressed (active high)
    
    while (1) {
        bool current_button_state = gpio_get_level(BUTTON_GPIO);
        
        // Button pressed (active high)
        if (current_button_state && !last_button_state) {
            if (!audio_session_is_recording()) {
                // Check if we have auth token from MQTT
                const char* auth_token = mqtt_get_auth_token();
                if (!auth_token || strlen(auth_token) == 0) {
                    ESP_LOGW(TAG, "Button pressed but no auth token available - waiting for MQTT setup");
                    led_set_system_state(LED_SYS_AUTH_WAITING);
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    led_set_system_state(LED_SYS_READY);
                } else {
                    // Start recording
                    led_set_system_state(LED_SYS_BUTTON_PRESSED);
                    esp_err_t ret = audio_session_start_recording();
                    if (ret == ESP_OK) {
                        // Send start message to server
                        const char* session_id = get_session_id();
                        ret = websocket_send_start_message(session_id, auth_token);
                        if (ret != ESP_OK) {
                            ESP_LOGE(TAG, "Failed to send start message");
                        }
                    }
                }
            }
        }
        
        // Button released (active low)
        if (!current_button_state && last_button_state) {
            if (audio_session_is_recording()) {
                // Stop recording
                esp_err_t ret = audio_session_stop_recording();
                if (ret == ESP_OK) {
                    // Send stop message to server
                    const char* session_id = get_session_id();
                    ret = websocket_send_stop_message(session_id);
                    if (ret != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to send stop message");
                    }
                }
            }
        }
        
        last_button_state = current_button_state;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

esp_err_t button_handler_init(void)
{
    ESP_LOGI(TAG, "Initializing button handler");
    
    // Configure button GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure button GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create button task
    BaseType_t task_ret = xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create button task");
        return ESP_FAIL;
    }
    
    return ESP_OK;
} 