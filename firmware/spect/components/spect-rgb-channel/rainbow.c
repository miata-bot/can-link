#include "spect-rgb-channel.h"
#include "rainbow.h"

rainbow_state_t rainbow_state;
rainbow_loop_t  loop_state;
rgb_t color = {.red=0, .green=0, .blue=0};

void rainbow_state_init(spect_rgb_t* rgb, uint16_t num_steps, uint16_t delay)
{
  rainbow_state.effects = 1;
  rainbow_loop_init(&loop_state, 1, false, 1);
  rainbow_state_reset(&rainbow_state, num_steps, delay);
  spect_rgb_fill(rgb, 0, rgb->strip->length, color);
  spect_rgb_blit(rgb);
  spect_rgb_wait(rgb);
}

void rainbow_loop(spect_rgb_t* rgb) {
  if(rainbow_step_loop(rgb) & 0x01) {
    spect_rgb_blit(rgb);
    spect_rgb_wait(rgb);
  }
}

void rainbow_loop_init(rainbow_loop_t* loop, uint8_t totchilds, bool timebased, uint16_t tottime) {
  loop->currentTime=0;
  loop->currentChild=0;
  loop->childs=totchilds;
  loop->timeBased=timebased;
  loop->cycles=tottime;
}

void rainbow_state_reset(rainbow_state_t* rainbow_state, uint16_t num_steps, uint16_t delay)
{
  rainbow_state->effStep = 0;
  rainbow_state->effect = (rainbow_state->effect + 1) % rainbow_state->effects;
  rainbow_state->effStart = esp_timer_get_time();
  rainbow_state->numSteps = num_steps;
  rainbow_state->delay = delay;
}

uint8_t rainbow_step_loop(spect_rgb_t* rgb) {
  uint8_t ret = 0x00;
  switch(loop_state.currentChild) {
    case 0: 
           ret = rainbow_step(rgb);break;
  }
  if(ret & 0x02) {
    ret &= 0xfd;
    if(loop_state.currentChild + 1 >= loop_state.childs) {
      loop_state.currentChild = 0;
      if(++loop_state.currentTime >= loop_state.cycles) {loop_state.currentTime = 0; ret |= 0x02;}
    }
    else {
      loop_state.currentChild++;
    }
  };
  return ret;
}

uint8_t rainbow_step(spect_rgb_t* rgb) {
  // Strip ID: 0 - Effect: Rainbow - LEDS: 300
  // Steps: 300 - Delay: 20
  // Colors: 3 (255.0.0, 0.255.0, 0.0.255)
  // Options: rainbowlen=300, toLeft=false, 
  if(esp_timer_get_time() - rainbow_state.effStart < 20 * (rainbow_state.effStep)) return 0x00;
  float factor1, factor2;
  uint16_t ind;
  rgb_t color;
  for(uint16_t j=0;j<300;j++) {
    ind = 300 - (uint16_t)(rainbow_state.effStep - j * 1) % 300;
    switch((int)((ind % 300) / 100)) {
      case 0: factor1 = 1.0 - ((float)(ind % 300 - 0 * 100) / 100);
              factor2 = (float)((int)(ind - 0) % 300) / 100;
              color.green = 255 * factor1 + 0 * factor2;
              color.red = 0 * factor1 + 255 * factor2;
              color.blue = 0 * factor1 + 0 * factor2;
              spect_rgb_set_pixel(rgb, j, color);
              spect_rgb_set_color(rgb, color);
              break;
      case 1: factor1 = 1.0 - ((float)(ind % 300 - 1 * 100) / 100);
              factor2 = (float)((int)(ind - 100) % 300) / 100;
              color.green = 0 * factor1 + 0 * factor2;
              color.red = 255 * factor1 + 0 * factor2;
              color.blue = 0 * factor1 + 255 * factor2;
              spect_rgb_set_pixel(rgb, j, color);
              spect_rgb_set_color(rgb, color);
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 300 - 2 * 100) / 100);
              factor2 = (float)((int)(ind - 200) % 300) / 100;
              color.green = 0 * factor1 + 255 * factor2;
              color.red = 0 * factor1 + 0 * factor2;
              color.blue = 255 * factor1 + 0 * factor2;
              spect_rgb_set_pixel(rgb, j, color);
              spect_rgb_set_color(rgb, color);
              break;
    }
  }
  if(rainbow_state.effStep >= 300) {rainbow_state_reset(&rainbow_state, rainbow_state.numSteps, rainbow_state.delay); return 0x03; }
  else rainbow_state.effStep++;
  return 0x01;
}
