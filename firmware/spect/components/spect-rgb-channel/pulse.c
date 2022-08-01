#include "pulse.h"
#include "esp_timer.h"

#define STRIP_NUM_LEDS 150
static rgb_t off   = {.green=0x00, .red = 0x00, .blue = 0x00};
static rgb_t color = {.green=0xff, .red = 0x00, .blue = 0xff};

static uint64_t start = 500;
static int8_t direction = 10;

void pulse_init(spect_rgb_t* rgb, uint32_t on_color, uint32_t off_color)
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

  for(uint32_t i = 0; i < rgb->strip->length; i++)
    spect_rgb_set_pixel(rgb, i, off);

  spect_rgb_blit(rgb);
  spect_rgb_wait(rgb);
}

void pulse_loop(spect_rgb_t* rgb)
{
  if((esp_timer_get_time() - start) >= 5000) {direction*=-1;};
  start = esp_timer_get_time();

  off.red+=direction;
  if(off.red >= color.red) off.red = (direction == 1) ? color.red : 0;

  off.green+=direction;
  if(off.green >= color.green) off.green = (direction == 1) ? color.green : 0;

  off.blue+=direction;
  if(off.blue >= color.blue) off.blue = (direction == 1) ? color.blue : 0;

  spect_rgb_fill(rgb, 0, rgb->strip->length, off);
  spect_rgb_blit(rgb);
  spect_rgb_wait(rgb);
  spect_rgb_set_color(rgb, off);
}