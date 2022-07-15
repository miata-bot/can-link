#ifndef SPECT_RGB_CHANNEL_H
#define SPECT_RGB_CHANNEL_H

#include <stdint.h>

#include <driver/gpio.h>
#include <driver/ledc.h>

#include <led_strip.h>

#include "freertos/semphr.h"

#define LEDC_CHANNEL_R 0
#define LEDC_CHANNEL_G 1
#define LEDC_CHANNEL_B 2

typedef struct SpectRGBConfig {
  uint16_t num_leds;
  gpio_num_t led_strip_gpio;
  uint8_t ledc_channel_offset;

  gpio_num_t led_red_gpio;
  gpio_num_t led_green_gpio;
  gpio_num_t led_blue_gpio;
} spect_rgb_config_t;

typedef struct SpectRGB {
  spect_rgb_config_t* cfg;
  led_strip_t* strip;
  ledc_channel_config_t ledc_channels[3];
  SemaphoreHandle_t counting_sem;
} spect_rgb_t;

/**
 * @brief initialize the global state of the rgb strips
 * 
 * @return esp_err_t 
 */
esp_err_t spect_rgb_install();

/**
 * @brief initialize a rgb channel
 * 
 * @return esp_err_t 
 */
esp_err_t spect_rgb_initialize(spect_rgb_config_t* cfg, spect_rgb_t** out_ctx);

esp_err_t spect_rgb_enable_pwm(spect_rgb_t* ctx);
esp_err_t spect_rgb_disable_pwm(spect_rgb_t* ctx);

esp_err_t spect_rgb_enable_strip(spect_rgb_t* ctx);
esp_err_t spect_rgb_disable_strip(spect_rgb_t* ctx);

/**
 * @brief set the color of the strip
 * 
 * @return esp_err_t 
 */
esp_err_t spect_rgb_set_color(spect_rgb_t* ctx, rgb_t color);

esp_err_t spect_rgb_set_pixel(spect_rgb_t* ctx, uint16_t address, rgb_t color);
esp_err_t spect_rgb_fill(spect_rgb_t* ctx, uint16_t start, uint16_t end, rgb_t color);
esp_err_t spect_rgb_blit(spect_rgb_t* ctx);
esp_err_t spect_rgb_wait(spect_rgb_t* ctx);

#endif