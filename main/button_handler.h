#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUTTON_GPIO GPIO_NUM_3  // Using GPIO3 as button input

/**
 * @brief Initialize button handler
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t button_handler_init(void);

/**
 * @brief Button task (internal use)
 */
void button_task(void *arg);

#endif // BUTTON_HANDLER_H 