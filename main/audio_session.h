#ifndef AUDIO_SESSION_H
#define AUDIO_SESSION_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

/**
 * @brief Initialize audio session
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t audio_session_init(void);

/**
 * @brief Start audio recording session
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t audio_session_start_recording(void);

/**
 * @brief Stop audio recording session
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t audio_session_stop_recording(void);

/**
 * @brief Check if currently recording
 * @return true if recording, false otherwise
 */
bool audio_session_is_recording(void);

/**
 * @brief Audio read task (internal use)
 */
void audio_read_task(void *arg);

/**
 * @brief Voice command callback (internal use)
 */
void voice_command_callback(char *command, uint16_t len, void *arg);

#endif // AUDIO_SESSION_H 