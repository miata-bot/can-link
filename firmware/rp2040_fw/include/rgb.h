#ifndef CONEPROJ_RGB_H
#define CONEPROJ_RGB_H
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "logging.h"

typedef struct RGB {
  uint32_t gpio_r;
  uint32_t gpio_g;
  uint32_t gpio_b;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t brightness;
} RGB_t;

void rgb_init(RGB_t*);
void rgb_enable(RGB_t*);
void rgb_disable(RGB_t*);
void rgb_set_color(RGB_t*, uint8_t, uint8_t, uint8_t);
void rgb_set_brightness(RGB_t*, uint8_t);
void rgb_set(RGB_t*);

#endif