#include "websocket_client.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "led.h"
#include "opus_wapper.h"
#include "vb6824.h"
#include <arpa/inet.h>
#include <inttypes.h>

#define TAG "WEBSOCKET_CLIENT"
#define READ_TASK_STACK_SIZE (8*1024)

static esp_websocket_client_handle_t ws_client = NULL;
static bool ws_connected = false;
static OpusCodec *s_opus_codec = NULL;

// Reverted and modified function to handle Opus from server
static void handle_received_opus_from_server(const uint8_t *data, size_t len) {
    if (len < 4) {
        ESP_LOGE(TAG, "Received Opus frame too short, needs at least 4 bytes for length header");
        return;
    }

    // Extract Opus frame length from header (first 4 bytes, big-endian)
    uint32_t opus_frame_len_from_header;
    memcpy(&opus_frame_len_from_header, data, 4);
    opus_frame_len_from_header = ntohl(opus_frame_len_from_header); // Convert from network byte order

    // Check if the total length matches the header-indicated length plus the header itself
    if (len != (opus_frame_len_from_header + 4)) {
        ESP_LOGW(TAG, "Received Opus frame length mismatch: expected %" PRIu32 " + 4, got %zu", opus_frame_len_from_header, len);
        // We will still attempt to decode using the length from the header
    }

    if (opus_frame_len_from_header > MAX_FRAME_SIZE) {
        ESP_LOGE(TAG, "Received Opus frame data too large: %" PRIu32 ", max is %d", opus_frame_len_from_header, MAX_FRAME_SIZE);
        return;
    }

    // Decode Opus data (which starts after the 4-byte header)
    static int16_t s_pcm_data[FRAME_SIZE] __attribute__((aligned(16)));
    
    int frame_size = opus_codec_decode(s_opus_codec, data + 4, opus_frame_len_from_header, s_pcm_data);
    if (frame_size > 0) {
        vb6824_audio_write((uint8_t *)s_pcm_data, frame_size * 2);
        ESP_LOGD(TAG, "Played decoded Opus frame: %d samples", frame_size);
    } else {
        ESP_LOGE(TAG, "Failed to decode Opus data from server: %d", frame_size);
    }
}

void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "✅ WEBSOCKET_EVENT_CONNECTED to %s", WS_URI);
            ws_connected = true;
            led_set_system_state(LED_SYS_WS_CONNECTED);
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "❌ WEBSOCKET_EVENT_DISCONNECTED");
            ws_connected = false;
            led_set_system_state(LED_SYS_ERROR);
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "❌ WEBSOCKET_EVENT_ERROR");
            ws_connected = false;
            led_set_system_state(LED_SYS_ERROR);
            break;
        case WEBSOCKET_EVENT_DATA:
            if (data->op_code == WS_TRANSPORT_OPCODES_TEXT) {
                ESP_LOGI(TAG, "📨 Received server JSON: %.*s", data->data_len, (char *)data->data_ptr);
                
                // Parse JSON to check for stop_stream command
                char json_str[256];
                if (data->data_len < sizeof(json_str) - 1) {
                    memcpy(json_str, data->data_ptr, data->data_len);
                    json_str[data->data_len] = '\0';
                    
                    ESP_LOGI(TAG, "Parsing JSON: %s", json_str);
                    
                    // Check if it's a stop_stream message - try multiple patterns
                    if (strstr(json_str, "stop_stream") != NULL ) {
                        // ESP_LOGI(TAG, "Found stop_stream command - turning LED OFF");
                        led_set_system_state(LED_SYS_IDLE);
                    }
                }
            } else if (data->op_code == WS_TRANSPORT_OPCODES_BINARY) {
                // ESP_LOGI(TAG, "🎵 Received binary audio data: %d bytes", data->data_len);
                led_set_system_state(LED_SYS_PLAYING);
                // Assume server sends Opus-encoded audio, decode and play
                handle_received_opus_from_server((const uint8_t *)data->data_ptr, data->data_len);
            }
            break;
        default:
            ESP_LOGD(TAG, "Other WebSocket event: %" PRId32, event_id);
            break;
    }
}

esp_err_t websocket_client_init(void)
{
    // Initialize Opus codec
    s_opus_codec = opus_codec_init(SAMPLE_RATE, SAMPLE_RATE, FRAME_DURATION, CHANNELS);
    if (s_opus_codec == NULL) {
        ESP_LOGE(TAG, "Failed to initialize Opus codec");
        return ESP_FAIL;
    }

    // Initialize WebSocket client with improved configuration
    esp_websocket_client_config_t ws_cfg = {
        .uri = WS_URI,
        .task_prio = 5,
        .task_stack = READ_TASK_STACK_SIZE,
        .buffer_size = 1024,
        .network_timeout_ms = 10000,
        .reconnect_timeout_ms = 10000,
        .disable_auto_reconnect = false,
        .skip_cert_common_name_check = true,
        .use_global_ca_store = false,
        .cert_pem = NULL,
        .cert_len = 0,
        .client_cert = NULL,
        .client_cert_len = 0,
        .client_key = NULL,
        .client_key_len = 0,
        .transport = WEBSOCKET_TRANSPORT_OVER_TCP,
        .subprotocol = NULL,
        .user_agent = NULL,
        .headers = NULL,
        .path = NULL,  // Remove path - connect to root
        .disable_pingpong_discon = false,
        .ping_interval_sec = 10,
        .pingpong_timeout_sec = 20,
    };
    
    ws_client = esp_websocket_client_init(&ws_cfg);
    if (ws_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return ESP_FAIL;
    }
    
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
    
    return ESP_OK;
}

esp_err_t websocket_client_start(void)
{
    if (ws_client == NULL) {
        ESP_LOGE(TAG, "WebSocket client not initialized");
        return ESP_FAIL;
    }
    
    led_set_system_state(LED_SYS_WS_CONNECTING);
    return esp_websocket_client_start(ws_client);
}

esp_err_t websocket_client_stop(void)
{
    if (ws_client == NULL) {
        return ESP_OK;
    }
    
    return esp_websocket_client_stop(ws_client);
}

esp_err_t websocket_send_start_message(const char* session_id, const char* auth_token)
{
    if (!ws_connected || !ws_client) {
        ESP_LOGE(TAG, "Cannot send start message: WebSocket not connected (connected=%d, client=%p)", ws_connected, ws_client);
        return ESP_FAIL;
    }
    
    if (!auth_token || strlen(auth_token) == 0) {
        ESP_LOGE(TAG, "Cannot send start message: No auth token available");
        return ESP_FAIL;
    }
    
    char json_str[512];  // Increased buffer size for auth token
    snprintf(json_str, sizeof(json_str),
             "{\"type\":\"start\",\"session_id\":\"%s\",\"sample_rate\":%d,\"channels\":%d,\"frame_duration\":%.1f,\"auth_token\":\"%s\"}",
             session_id, SAMPLE_RATE, CHANNELS, (float)FRAME_DURATION, auth_token);
    
    int result = esp_websocket_client_send_text(ws_client, json_str, strlen(json_str), portMAX_DELAY);
    if (result < 0) {
        ESP_LOGE(TAG, "Failed to send start message: %d", result);
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "📤 Sent start message: %s", json_str);
        return ESP_OK;
    }
}

esp_err_t websocket_send_stop_message(const char* session_id)
{
    if (!ws_connected || !ws_client) {
        ESP_LOGE(TAG, "Cannot send stop message: WebSocket not connected (connected=%d, client=%p)", ws_connected, ws_client);
        return ESP_FAIL;
    }
    
    char json_str[128];
    snprintf(json_str, sizeof(json_str),
             "{\"type\":\"stop\",\"session_id\":\"%s\"}",
             session_id);
    
    int result = esp_websocket_client_send_text(ws_client, json_str, strlen(json_str), portMAX_DELAY);
    if (result < 0) {
        ESP_LOGE(TAG, "Failed to send stop message: %d", result);
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "📤 Sent stop message: %s", json_str);
        return ESP_OK;
    }
}

esp_err_t websocket_send_audio_frame(const uint8_t* data, size_t len)
{
    if (!ws_connected || !ws_client) {
        ESP_LOGW(TAG, "WebSocket not connected, skipping audio frame");
        return ESP_FAIL;
    }
    
    // Create a buffer for the complete frame (4 bytes header + data)
    uint8_t frame_buffer[MAX_FRAME_SIZE + 4];
    // Add frame length header (4 bytes) in big-endian format
    uint32_t frame_len = htonl(len);
    memcpy(frame_buffer, &frame_len, 4);
    // Copy the Opus data after the header
    memcpy(frame_buffer + 4, data, len);
    // Send the complete frame
    int result = esp_websocket_client_send_bin(ws_client, (const char *)frame_buffer, len + 4, portMAX_DELAY);
    if (result < 0) {
        ESP_LOGE(TAG, "Failed to send audio frame: %d", result);
        return ESP_FAIL;
    } else {
        ESP_LOGD(TAG, "📤 Sent audio frame: %d bytes", len + 4);
        return ESP_OK;
    }
}

bool websocket_is_connected(void) {
    return ws_connected;
}

esp_websocket_client_handle_t websocket_get_client(void) {
    return ws_client;
} 