#ifndef LED_H
#define LED_H

#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// RGB LED GPIO pins
#define LED_RED_GPIO   GPIO_NUM_0
#define LED_GREEN_GPIO GPIO_NUM_1  
#define LED_BLUE_GPIO  GPIO_NUM_2

// LED states
typedef enum {
    LED_OFF = 0,
    LED_RED,
    LED_GREEN, 
    LED_BLUE,
    LED_YELLOW,  // Red + Green
    LED_CYAN,    // Green + Blue
    LED_MAGENTA, // Red + Blue
    LED_WHITE    // All colors
} led_state_t;

// LED animation patterns
typedef enum {
    LED_ANIM_NONE = 0,
    LED_ANIM_BLINK,      // Simple blink
    LED_ANIM_PULSE,      // Fade in/out
    LED_ANIM_ROTATE,     // Rotate through colors
    LED_ANIM_BREATH,     // Breathing effect
    LED_ANIM_WAVE,       // Wave pattern
    LED_ANIM_LOADING     // Loading animation
} led_animation_t;

// LED system states
typedef enum {
    LED_SYS_IDLE = 0,           // System idle - Green solid
    LED_SYS_WIFI_CONNECTING,    // WiFi connecting - Yellow blink
    LED_SYS_WIFI_CONNECTED,     // WiFi connected - Green solid
    LED_SYS_WS_CONNECTING,      // WebSocket connecting - Cyan blink
    LED_SYS_WS_CONNECTED,       // WebSocket connected - Green solid
    LED_SYS_AUTH_WAITING,       // Waiting for auth - Yellow pulse
    LED_SYS_READY,              // Ready to record - Green solid
    LED_SYS_BUTTON_PRESSED,     // Button pressed - Magenta solid
    LED_SYS_RECORDING,          // Recording - Magenta pulse
    LED_SYS_PLAYING,            // Playing audio - Blue pulse
    LED_SYS_ERROR               // Error state - Red blink
} led_system_state_t;

/**
 * @brief Initialize RGB LED
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_init(void);

/**
 * @brief Set LED color
 * @param color LED color to set
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_set_color(led_state_t color);

/**
 * @brief Turn off all LEDs
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_off(void);

/**
 * @brief Set individual LED states
 * @param red Red LED state (0=off, 1=on)
 * @param green Green LED state (0=off, 1=on)
 * @param blue Blue LED state (0=off, 1=on)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_set_rgb(bool red, bool green, bool blue);

/**
 * @brief Start LED animation
 * @param animation Animation type to start
 * @param color Base color for animation
 * @param speed Animation speed in ms
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_start_animation(led_animation_t animation, led_state_t color, uint32_t speed);

/**
 * @brief Stop LED animation
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_stop_animation(void);

/**
 * @brief Set system state (automatically handles animations)
 * @param state System state to set
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_set_system_state(led_system_state_t state);

/**
 * @brief Get current system state
 * @return Current system state
 */
led_system_state_t led_get_system_state(void);

/**
 * @brief LED animation task (internal use)
 */
void led_animation_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // LED_H 