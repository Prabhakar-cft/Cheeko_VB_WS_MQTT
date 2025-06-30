#include "session_manager.h"
#include "led.h"
#include "mqtt_handler.h"
#include "wifi_manager.h"
#include "websocket_client.h"
#include "audio_session.h"
#include "button_handler.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG "MAIN"

// MQTT connection callback
static void mqtt_connection_callback(bool connected) {
    if (connected) {
        ESP_LOGI(TAG, "MQTT connected successfully");
        led_set_system_state(LED_SYS_READY);
    } else {
        ESP_LOGW(TAG, "MQTT disconnected");
        led_set_system_state(LED_SYS_ERROR);
    }
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize session manager (generates session ID)
    session_manager_init();

    // Initialize LED
    ret = led_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED: %s", esp_err_to_name(ret));
    } else {
        led_set_system_state(LED_SYS_IDLE);
    }

    // Initialize MQTT handler
    ret = mqtt_handler_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQTT handler: %s", esp_err_to_name(ret));
        led_set_system_state(LED_SYS_ERROR);
        return;
    }
    
    // Register MQTT connection callback
    mqtt_register_connection_callback(mqtt_connection_callback);

    // Initialize WiFi
    wifi_manager_init();

    // Initialize WebSocket client
    websocket_client_init();
    websocket_client_start();

    // Initialize audio session
    audio_session_init();

    // Initialize button handler
    button_handler_init();
} 