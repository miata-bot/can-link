#ifndef CONEPROJ_PICO_H
#define CONEPROJ_PICO_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <sdkconfig.h>

typedef struct {
    spi_host_device_t host;
    gpio_num_t gpio_cs;
    gpio_num_t gpio_en;
} pico_config_t;

typedef struct pico {
    pico_config_t cfg;
    spi_device_handle_t spi;
} pico_t;

typedef enum __attribute__((__packed__)) {
  COMMAND_SYNC                   = 0xff,
  COMMAND_STATUS_LED_SET_STATE   = 1,
  COMMAND_STATUS_LED_BLINK       = 2,
  COMMAND_REGULATOR_SET_STATE    = 3,
  COMMAND_RGB_SET_COLOR          = 4,
  COMMAND_RGB_SET_BRIGHTNESS     = 5,
  COMMAND_RGB_ENABLE             = 6,
  COMMAND_RGB_DISABLE            = 7,
} command_type_t;

typedef enum __attribute__((__packed__)) {
  COMMAND_STATUS_LED_INDEX_GREEN = 1,
  COMMAND_STATUS_LED_INDEX_RED   = 2,
  COMMAND_REGULATOR_INDEX_EN     = 3,
  COMMAND_RGB_INDEX_0            = 4,
  COMMAND_RGB_INDEX_1            = 5,
} command_arg_index_t;

typedef enum __attribute__((__packed__)) {
  COMMAND_RESPONSE_DESYNC                = 0xff,
  COMMAND_RESPONSE_BUSY                  = 0xfe,
  COMMAND_RESPONSE_OK                    = 0xfb,
  COMMAND_RESPONSE_ERROR_NO_RESP         = 0x0,
  COMMAND_RESPONSE_ERROR_BAD_INDEX       = 0x1,
  COMMAND_RESPONSE_ERROR_UNKNOWN_COMMAND = 0x2,
} command_response_t;

typedef enum __attribute__((__packed__)) {
  STATUS_HIGH = 1,
  STATUS_LOW  = 2,
} status_led_state_t;

typedef struct __attribute__((__packed__)) command_status_led_set_state_args {
  status_led_state_t state;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} command_status_led_set_state_args_t;
// static_assert(sizeof(command_status_led_set_state_args_t) == 3);

typedef struct __attribute__((__packed__)) command_status_led_blink_args {
  uint8_t blink_ms;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} command_status_led_blink_args_t;
// static_assert(sizeof(command_status_led_blink_args_t) == 3);

typedef enum __attribute__((__packed__))  {
  REGULATOR_ON = 1,
  REGULATOR_OFF = 0
} regulator_state_t;

typedef struct __attribute__((__packed__)) regulator_set_state_args {
  regulator_state_t state;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} regulator_set_state_args_t;
// static_assert(sizeof(regulator_set_state_args_t) == 3);

typedef struct __attribute__((__packed__)) rgb_set_color_args {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb_set_color_args_t;
// static_assert(sizeof(rgb_set_color_args_t) == 3);

typedef struct __attribute__((__packed__)) rgb_set_brightness_args {
  uint8_t brightness;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} rgb_set_brightness_args_t;
// static_assert(sizeof(rgb_set_brightness_args_t) == 3);

typedef struct __attribute__((__packed__)) rgb_enable_args {
  uint8_t _unusedarg2;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} rgb_enable_args_t;
// static_assert(sizeof(rgb_enable_args_t) == 3);

typedef struct __attribute__((__packed__))  rgb_disable_args {
  uint8_t _unusedarg2;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} rgb_disable_args_t;
// static_assert(sizeof(rgb_disable_args_t) == 3);

// Must be 24 bits total (3 bytes).
typedef union __attribute__((__packed__)) {
  command_status_led_set_state_args_t status_led_set_state;
  command_status_led_blink_args_t     status_led_blink;
  regulator_set_state_args_t          regulator_set_state;
  rgb_set_color_args_t                rgb_set_color;
  rgb_set_brightness_args_t           rgb_set_brightness;
  rgb_enable_args_t                   rgb_enable;
  rgb_disable_args_t                  rgb_disable;
  uint8_t                             _raw[3];
} command_args_t;
// static_assert(sizeof(command_args_t) == 4);

typedef struct {
  command_type_t type;
  command_arg_index_t index;
  command_args_t args;
  uint8_t rx_buffer[5];
} pico_command_t;

esp_err_t pico_initialize(pico_config_t *, pico_t**);
esp_err_t pico_sync(pico_t*);
esp_err_t pico_reset(pico_t*);

command_response_t pico_send_command(pico_t*, pico_command_t*);

command_response_t pico_ping(pico_t*);
command_response_t pico_set_color(pico_t*, command_arg_index_t, uint8_t, uint8_t, uint8_t);
command_response_t pico_set_brightness(pico_t*, command_arg_index_t, uint8_t);

#endif