#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "esp_event.h"
#include <stdbool.h>

// WiFi credentials
#define WIFI_SSID "craftech360"
#define WIFI_PASS "cftCFT360"

/**
 * @brief Initialize WiFi station mode
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Get WiFi connection status
 * @return true if connected, false otherwise
 */
bool wifi_manager_is_connected(void);

/**
 * @brief WiFi event handler (internal use)
 */
void wifi_event_handler(void* arg, esp_event_base_t event_base,
                       int32_t event_id, void* event_data);

#endif // WIFI_MANAGER_H 