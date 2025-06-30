#include "led.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <math.h>
#include <inttypes.h>

#define TAG "LED"
#define LED_ANIMATION_TASK_STACK_SIZE 2048
#define LED_ANIMATION_TASK_PRIORITY 5

// Animation control
static TaskHandle_t led_animation_task_handle = NULL;
static SemaphoreHandle_t led_mutex = NULL;
static led_animation_t current_animation = LED_ANIM_NONE;
static led_state_t animation_color = LED_OFF;
static uint32_t animation_speed = 500;
static bool animation_running = false;
static led_system_state_t current_system_state = LED_SYS_IDLE;

// Animation state
static uint32_t animation_counter = 0;
static bool animation_state = false;

/**
 * @brief Initialize RGB LED
 */
esp_err_t led_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_RED_GPIO) | (1ULL << LED_GREEN_GPIO) | (1ULL << LED_BLUE_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LED GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create mutex for thread safety
    led_mutex = xSemaphoreCreateMutex();
    if (led_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create LED mutex");
        return ESP_FAIL;
    }
    
    // Turn off all LEDs initially
    gpio_set_level(LED_RED_GPIO, 0);
    gpio_set_level(LED_GREEN_GPIO, 0);
    gpio_set_level(LED_BLUE_GPIO, 0);
    
    // Create animation task
    BaseType_t task_ret = xTaskCreate(led_animation_task, "led_anim", 
                                     LED_ANIMATION_TASK_STACK_SIZE, NULL, 
                                     LED_ANIMATION_TASK_PRIORITY, &led_animation_task_handle);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LED animation task");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "RGB LED initialized on GPIO %d, %d, %d", LED_RED_GPIO, LED_GREEN_GPIO, LED_BLUE_GPIO);
    return ESP_OK;
}

/**
 * @brief Set LED color
 */
esp_err_t led_set_color(led_state_t color) {
    if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    switch (color) {
        case LED_OFF:
            gpio_set_level(LED_RED_GPIO, 0);
            gpio_set_level(LED_GREEN_GPIO, 0);
            gpio_set_level(LED_BLUE_GPIO, 0);
            ESP_LOGD(TAG, "LED set to OFF");
            break;
        case LED_RED:
            gpio_set_level(LED_RED_GPIO, 1);
            gpio_set_level(LED_GREEN_GPIO, 0);
            gpio_set_level(LED_BLUE_GPIO, 0);
            ESP_LOGD(TAG, "LED set to RED");
            break;
        case LED_GREEN:
            gpio_set_level(LED_RED_GPIO, 0);
            gpio_set_level(LED_GREEN_GPIO, 1);
            gpio_set_level(LED_BLUE_GPIO, 0);
            ESP_LOGD(TAG, "LED set to GREEN");
            break;
        case LED_BLUE:
            gpio_set_level(LED_RED_GPIO, 0);
            gpio_set_level(LED_GREEN_GPIO, 0);
            gpio_set_level(LED_BLUE_GPIO, 1);
            ESP_LOGD(TAG, "LED set to BLUE");
            break;
        case LED_YELLOW:
            gpio_set_level(LED_RED_GPIO, 1);
            gpio_set_level(LED_GREEN_GPIO, 1);
            gpio_set_level(LED_BLUE_GPIO, 0);
            ESP_LOGD(TAG, "LED set to YELLOW");
            break;
        case LED_CYAN:
            gpio_set_level(LED_RED_GPIO, 0);
            gpio_set_level(LED_GREEN_GPIO, 1);
            gpio_set_level(LED_BLUE_GPIO, 1);
            ESP_LOGD(TAG, "LED set to CYAN");
            break;
        case LED_MAGENTA:
            gpio_set_level(LED_RED_GPIO, 1);
            gpio_set_level(LED_GREEN_GPIO, 0);
            gpio_set_level(LED_BLUE_GPIO, 1);
            ESP_LOGD(TAG, "LED set to MAGENTA");
            break;
        case LED_WHITE:
            gpio_set_level(LED_RED_GPIO, 1);
            gpio_set_level(LED_GREEN_GPIO, 1);
            gpio_set_level(LED_BLUE_GPIO, 1);
            ESP_LOGD(TAG, "LED set to WHITE");
            break;
        default:
            ESP_LOGE(TAG, "Invalid LED color: %d", color);
            xSemaphoreGive(led_mutex);
            return ESP_ERR_INVALID_ARG;
    }
    
    xSemaphoreGive(led_mutex);
    return ESP_OK;
}

/**
 * @brief Turn off all LEDs
 */
esp_err_t led_off(void) {
    return led_set_color(LED_OFF);
}

/**
 * @brief Set individual LED states
 */
esp_err_t led_set_rgb(bool red, bool green, bool blue) {
    if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    gpio_set_level(LED_RED_GPIO, red ? 1 : 0);
    gpio_set_level(LED_GREEN_GPIO, green ? 1 : 0);
    gpio_set_level(LED_BLUE_GPIO, blue ? 1 : 0);
    
    ESP_LOGD(TAG, "LED RGB set to R:%d G:%d B:%d", red, green, blue);
    
    xSemaphoreGive(led_mutex);
    return ESP_OK;
}

/**
 * @brief Start LED animation
 */
esp_err_t led_start_animation(led_animation_t animation, led_state_t color, uint32_t speed) {
    if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    current_animation = animation;
    animation_color = color;
    animation_speed = speed;
    animation_running = true;
    animation_counter = 0;
    animation_state = false;
    
    // ESP_LOGI(TAG, "Started LED animation: %d, color: %d, speed: %" PRIu32 "ms", animation, color, speed);
    
    xSemaphoreGive(led_mutex);
    return ESP_OK;
}

/**
 * @brief Stop LED animation
 */
esp_err_t led_stop_animation(void) {
    if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    animation_running = false;
    current_animation = LED_ANIM_NONE;
    
    ESP_LOGI(TAG, "Stopped LED animation");
    
    xSemaphoreGive(led_mutex);
    return ESP_OK;
}

/**
 * @brief Set system state (automatically handles animations)
 */
esp_err_t led_set_system_state(led_system_state_t state) {
    current_system_state = state;
    
    switch (state) {
        case LED_SYS_IDLE:
            led_stop_animation();
            led_set_color(LED_OFF);
            break;
            
        case LED_SYS_WIFI_CONNECTING:
            led_start_animation(LED_ANIM_BLINK, LED_YELLOW, 100);
            break;
            
        case LED_SYS_WIFI_CONNECTED:
            led_stop_animation();
            led_set_color(LED_GREEN);
            break;
            
        case LED_SYS_WS_CONNECTING:
            led_start_animation(LED_ANIM_BLINK, LED_CYAN, 300);
            break;
            
        case LED_SYS_WS_CONNECTED:
            led_stop_animation();
            led_set_color(LED_GREEN);
            break;
            
        case LED_SYS_AUTH_WAITING:
            led_start_animation(LED_ANIM_PULSE, LED_YELLOW, 1000);
            break;
            
        case LED_SYS_READY:
            led_stop_animation();
            led_set_color(LED_GREEN);
            break;
            
        case LED_SYS_BUTTON_PRESSED:
            led_stop_animation();
            led_set_color(LED_MAGENTA);
            break;
            
        case LED_SYS_RECORDING:
            led_start_animation(LED_ANIM_PULSE, LED_MAGENTA, 800);
            break;
            
        case LED_SYS_PLAYING:
            led_start_animation(LED_ANIM_PULSE, LED_BLUE, 600);
            break;
            
        case LED_SYS_ERROR:
            led_start_animation(LED_ANIM_BLINK, LED_RED, 200);
            break;
            
        default:
            ESP_LOGE(TAG, "Invalid system state: %d", state);
            return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "LED system state changed to: %d", state);
    return ESP_OK;
}

/**
 * @brief Get current system state
 */
led_system_state_t led_get_system_state(void) {
    return current_system_state;
}

/**
 * @brief LED animation task
 */
void led_animation_task(void *pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    
    while (1) {
        if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            if (animation_running) {
                switch (current_animation) {
                    case LED_ANIM_BLINK:
                        // Simple blink animation
                        if (animation_counter % 2 == 0) {
                            led_set_color(animation_color);
                        } else {
                            led_set_color(LED_OFF);
                        }
                        break;
                        
                    case LED_ANIM_PULSE:
                        // Pulse animation (simulated with blink for now)
                        if (animation_counter % 4 < 2) {
                            led_set_color(animation_color);
                        } else {
                            led_set_color(LED_OFF);
                        }
                        break;
                        
                    case LED_ANIM_ROTATE:
                        // Rotate through colors
                        led_state_t colors[] = {LED_RED, LED_GREEN, LED_BLUE, LED_YELLOW, LED_CYAN, LED_MAGENTA};
                        int color_index = (animation_counter / 2) % 6;
                        led_set_color(colors[color_index]);
                        break;
                        
                    case LED_ANIM_BREATH:
                        // Breathing effect (simulated)
                        if (animation_counter % 8 < 4) {
                            led_set_color(animation_color);
                        } else {
                            led_set_color(LED_OFF);
                        }
                        break;
                        
                    case LED_ANIM_WAVE:
                        // Wave pattern
                        led_state_t wave_colors[] = {LED_RED, LED_YELLOW, LED_GREEN, LED_CYAN, LED_BLUE, LED_MAGENTA};
                        int wave_index = (animation_counter / 3) % 6;
                        led_set_color(wave_colors[wave_index]);
                        break;
                        
                    case LED_ANIM_LOADING:
                        // Loading animation
                        if (animation_counter % 6 < 3) {
                            led_set_color(animation_color);
                        } else {
                            led_set_color(LED_OFF);
                        }
                        break;
                        
                    default:
                        break;
                }
                animation_counter++;
            }
            xSemaphoreGive(led_mutex);
        }
        
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(animation_speed));
    }
} 