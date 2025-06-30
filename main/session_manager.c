#include "session_manager.h"
#include "esp_log.h"
#include "esp_random.h"
#include <stdio.h>
#include <inttypes.h>

#define TAG "SESSION_MANAGER"

// Session control
static char session_id[37];  // UUID4 string length + null terminator

void generate_uuid4(char *uuid_str) {
    uint8_t uuid[16];
    for (int i = 0; i < 16; i++) {
        uuid[i] = (uint8_t)esp_random();
    }
    // Set version (4) and variant (10)
    uuid[6] = (uuid[6] & 0x0F) | 0x40; // Version 4
    uuid[8] = (uuid[8] & 0x3F) | 0x80; // Variant 10

    snprintf(uuid_str, 37,
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uuid[0], uuid[1], uuid[2], uuid[3],
        uuid[4], uuid[5],
        uuid[6], uuid[7],
        uuid[8], uuid[9],
        uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]
    );
}

const char* get_session_id(void) {
    return session_id;
}

esp_err_t session_manager_init(void) {
    // Generate a new UUID4 for this session
    generate_uuid4(session_id);
    ESP_LOGI(TAG, "Generated session ID: %s", session_id);
    return ESP_OK;
} 