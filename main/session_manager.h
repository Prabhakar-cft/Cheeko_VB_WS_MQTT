#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief Generate a UUID4 string (RFC 4122 compliant)
 * @param uuid_str Buffer to store the generated UUID string (must be at least 37 bytes)
 */
void generate_uuid4(char *uuid_str);

/**
 * @brief Get the current session ID
 * @return Pointer to the current session ID string
 */
const char* get_session_id(void);

/**
 * @brief Initialize session manager
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t session_manager_init(void);

#endif // SESSION_MANAGER_H 