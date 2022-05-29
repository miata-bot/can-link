#ifndef CONEPROJ_STATUS_LED_H
#define CONEPROJ_STATUS_LED_H

#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"

typedef enum __attribute__((__packed__)) {
  STATUS_HIGH = 1,
  STATUS_LOW  = 2,
} status_led_state_t;

typedef struct status_led {
  uint32_t gpio;
  status_led_state_t state;
} status_led_t;

void status_led_init(status_led_t*);

void status_led_blink(status_led_t*, uint32_t);

void status_led_set_state(status_led_t*, status_led_state_t);

#endif