#ifndef CONEPROJ_WS812_H
#define CONEPROJ_WS812_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "ws2812.pio.h"

typedef struct ws2812 {
  PIO pio;
  int sm;
  uint offset;
  uint32_t gpio;
  bool is_rgbw;
  uint8_t num_nodes;
  uint32_t frequency;
} ws2812_t;

void ws2812_init(ws2812_t*);
void ws2812_put_pixel(ws2812_t*, uint32_t);
void ws2812_pattern_snakes(ws2812_t*, uint, uint);
void ws2812_pattern_random(ws2812_t*, uint, uint);
void ws2812_pattern_sparkle(ws2812_t*, uint, uint);
void ws2812_pattern_greys(ws2812_t*, uint, uint);

#endif