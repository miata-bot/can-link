#ifndef CONEPROJ_COMMAND_H
#define CONEPROJ_COMMAND

#include <stdint.h>
#include <stdlib.h>

#include <assert.h>

#include "status_led.h"
#include "regulator.h"
#include "rgb.h"
#include "ws2812.h"

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
  COMMAND_RESPONSE_SYNC                  = 0xff,
  COMMAND_RESPONSE_BUSY                  = 0xfe,
  COMMAND_RESPONSE_OK                    = 0xfb,
  COMMAND_RESPONSE_ERROR_BAD_INDEX       = 0x1,
  COMMAND_RESPONSE_ERROR_UNKNOWN_COMMAND = 0x2,
} command_response_t;

typedef struct __attribute__((__packed__)) command_status_led_set_state_args {
  command_arg_index_t index;
  status_led_state_t state;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} command_status_led_set_state_args_t;
static_assert(sizeof(command_status_led_set_state_args_t) == 4);

typedef struct __attribute__((__packed__)) command_status_led_blink_args {
  command_arg_index_t index;
  uint8_t blink_ms;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} command_status_led_blink_args_t;
static_assert(sizeof(command_status_led_blink_args_t) == 4);

typedef struct __attribute__((__packed__)) regulator_set_state_args {
  command_arg_index_t index;
  regulator_state_t state;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} regulator_set_state_args_t;
static_assert(sizeof(regulator_set_state_args_t) == 4);

typedef struct __attribute__((__packed__)) rgb_set_color_args {
  command_arg_index_t index;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb_set_color_args_t;
static_assert(sizeof(rgb_set_color_args_t) == 4);

typedef struct __attribute__((__packed__)) rgb_set_brightness_args {
  command_arg_index_t index;
  uint8_t brightness;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} rgb_set_brightness_args_t;
static_assert(sizeof(rgb_set_brightness_args_t) == 4);

typedef struct __attribute__((__packed__)) rgb_enable_args {
  command_arg_index_t index;
  uint8_t _unusedarg2;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} rgb_enable_args_t;
static_assert(sizeof(rgb_enable_args_t) == 4);

typedef struct __attribute__((__packed__))  rgb_disable_args {
  command_arg_index_t index;
  uint8_t _unusedarg2;
  uint8_t _unusedarg3;
  uint8_t _unusedarg4;
} rgb_disable_args_t;
static_assert(sizeof(rgb_disable_args_t) == 4);

typedef union {
  command_status_led_set_state_args_t status_led_set_state;
  command_status_led_blink_args_t     status_led_blink;
  regulator_set_state_args_t          regulator_set_state;
  rgb_set_color_args_t                rgb_set_color;
  rgb_set_brightness_args_t           rgb_set_brightness;
  rgb_enable_args_t                   rgb_enable;
  rgb_disable_args_t                  rgb_disable;
} command_args_t;

typedef struct command {
  command_type_t type;
  command_args_t args;
  command_response_t response;

  spi_inst_t *spi;
  uint32_t gpio_rx;
  uint32_t gpio_tx;
  uint32_t gpio_sclk;
  uint32_t gpio_csn;

  // [command+index, arg0, arg1, arg3]
  uint8_t buffer[4];
} command_t;

void command_init(command_t*);
void command_read(command_t*);
void command_decode(command_t*);
void command_reply(command_t*);

#endif