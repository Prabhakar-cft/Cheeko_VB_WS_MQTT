#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include "esp_err.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include <stdbool.h>

#define WS_URI "ws://139.59.7.72:5008"

// Audio configuration
#define SAMPLE_RATE 16000
#define CHANNELS 1
#define FRAME_DURATION 60  // ms (60ms per frame)
#define FRAME_SIZE (SAMPLE_RATE * FRAME_DURATION / 1000)  // 960 samples for 60ms at 16kHz
#define MAX_FRAME_SIZE 1275  // Maximum Opus frame size

/**
 * @brief Initialize WebSocket client
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t websocket_client_init(void);

/**
 * @brief Start WebSocket client
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t websocket_client_start(void);

/**
 * @brief Stop WebSocket client
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t websocket_client_stop(void);

/**
 * @brief Send start message to server
 * @param session_id Session ID to include in message
 * @param auth_token Authentication token
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t websocket_send_start_message(const char* session_id, const char* auth_token);

/**
 * @brief Send stop message to server
 * @param session_id Session ID to include in message
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t websocket_send_stop_message(const char* session_id);

/**
 * @brief Send audio frame to server
 * @param data Audio data buffer
 * @param len Length of audio data
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t websocket_send_audio_frame(const uint8_t* data, size_t len);

/**
 * @brief Check if WebSocket is connected
 * @return true if connected, false otherwise
 */
bool websocket_is_connected(void);

/**
 * @brief Get WebSocket client handle
 * @return WebSocket client handle
 */
esp_websocket_client_handle_t websocket_get_client(void);

/**
 * @brief WebSocket event handler (internal use)
 */
void websocket_event_handler(void *handler_args, esp_event_base_t base, 
                           int32_t event_id, void *event_data);

#endif // WEBSOCKET_CLIENT_H 