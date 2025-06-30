#include "audio_session.h"
#include "esp_log.h"
#include "vb6824.h"
#include "websocket_client.h"
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "AUDIO_SESSION"
#define READ_TASK_STACK_SIZE (8*1024)

static bool is_recording = false;
static TaskHandle_t read_task_handle = NULL;

void voice_command_callback(char *command, uint16_t len, void *arg)
{
    ESP_LOGI(TAG, "Received voice command: %.*s\n", len, command);
}

void audio_read_task(void *arg) {
    // Align buffers to 16 bytes for better performance
    uint8_t data[MAX_FRAME_SIZE] __attribute__((aligned(16)));
    
    while (1) {
        if (is_recording) {
            uint16_t len = vb6824_audio_read(data, sizeof(data));
            if (len > 0) {
                // Send the Opus data through WebSocket if connected
                if (websocket_is_connected()) {
                    esp_err_t result = websocket_send_audio_frame(data, len);
                    if (result != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to send audio frame");
                    }
                } else {
                    ESP_LOGW(TAG, "WebSocket not connected, skipping audio frame");
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(100)); // Avoid busy waiting
        }
    }
}

esp_err_t audio_session_init(void)
{
    ESP_LOGI(TAG, "Initializing audio session");
    
    // Initialize VB6824 audio hardware
    vb6824_init(GPIO_NUM_10, GPIO_NUM_18);
    vb6824_register_voice_command_cb(voice_command_callback, NULL);
    vb6824_audio_enable_input(1);
    vb6824_audio_enable_output(1);
    
    // Create audio read task
    BaseType_t ret = xTaskCreate(audio_read_task, "audio_read_task", READ_TASK_STACK_SIZE, NULL, 10, &read_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create audio read task");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t audio_session_start_recording(void)
{
    if (is_recording) {
        ESP_LOGW(TAG, "Already recording");
        return ESP_OK;
    }
    
    is_recording = true;
    ESP_LOGI(TAG, "Starting audio recording session");
    led_set_system_state(LED_SYS_RECORDING);
    
    return ESP_OK;
}

esp_err_t audio_session_stop_recording(void)
{
    if (!is_recording) {
        ESP_LOGW(TAG, "Not currently recording");
        return ESP_OK;
    }
    
    is_recording = false;
    ESP_LOGI(TAG, "Stopping audio recording session");
    led_set_system_state(LED_SYS_READY);
    
    return ESP_OK;
}

bool audio_session_is_recording(void)
{
    return is_recording;
} 