#include "chase.h"
#include "esp_timer.h"

#define STRIP_NUM_LEDS 150
static rgb_t off   = {.green=0x00, .red = 0x00, .blue = 0x00};
static rgb_t color = {.green=0xff, .red = 0x00, .blue = 0xff};
// static rgb_t colors[] = {
//   {.green=0xff, .red = 0x00, .blue = 0},
//   {.green=0, .red = 0xff, .blue = 0},
//   {.green=0, .red = 0x00, .blue = 0xff},
//   {.green=0xff, .red = 0xff, .blue = 0xff},
// };
// uint8_t color_idx = 0;

static uint32_t current_led = 0;
static uint64_t start = 500;
int8_t direction = 1;

void chase_init(spect_rgb_t* rgb, uint32_t on_color, uint32_t off_color)
{
  uint8_t color_buffer[4] = {0};
  color_buffer[0] = on_color;
  color_buffer[1] = on_color >>  8;
  color_buffer[2] = on_color >> 16;
  color_buffer[3] = on_color >> 24;

  color.red   = color_buffer[1];
  color.green = color_buffer[0];
  color.blue  = color_buffer[2];

  color_buffer[0] = off_color;
  color_buffer[1] = off_color >>  8;
  color_buffer[2] = off_color >> 16;
  color_buffer[3] = off_color >> 24;

  off.red   = color_buffer[1];
  off.green = color_buffer[0];
  off.blue  = color_buffer[2];

  current_led = 1;
  for(uint32_t i = 0; i < rgb->strip->length; i++)
    spect_rgb_set_pixel(rgb, i, off);

  spect_rgb_blit(rgb);
  spect_rgb_wait(rgb);
}

void chase_loop(spect_rgb_t* rgb)
{
  if((esp_timer_get_time() - start) < 100) {
    return;
  }
  start = esp_timer_get_time();

  current_led+= direction;
  if((current_led == STRIP_NUM_LEDS) || (current_led == 0)) {
    direction*=-1;
  };

  spect_rgb_set_pixel(rgb, current_led-3, off);
  spect_rgb_set_pixel(rgb, current_led-2, color);
  spect_rgb_set_pixel(rgb, current_led-1, color);
  spect_rgb_set_pixel(rgb, current_led, color);
  spect_rgb_set_pixel(rgb, current_led+1, color);
  spect_rgb_set_pixel(rgb, current_led+2, color);
  spect_rgb_set_pixel(rgb, current_led+3, off);
  spect_rgb_blit(rgb);
  spect_rgb_wait(rgb);
  spect_rgb_set_color(rgb, off);
  // if(current_led % 7) color_idx++;
  // if(color_idx > 4) color_idx = 0;
}