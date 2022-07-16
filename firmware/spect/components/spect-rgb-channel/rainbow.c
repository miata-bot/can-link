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
  uint16_t num_steps = 146;
  float steps_per_color = num_steps / 3;
  uint16_t delay = 99;
  if(esp_timer_get_time() - rainbow_state.effStart < delay * (rainbow_state.effStep)) return 0x00;
  float factor1, factor2;
  uint16_t ind;
  rgb_t color;
  for(uint16_t j=0; j<rgb->strip->length; j++) {
    ind = num_steps - (uint16_t)(rainbow_state.effStep - j * 1) % num_steps;
    switch((int)((ind % num_steps) / steps_per_color)) {
      case 0: factor1 = 1.0 - ((float)(ind % num_steps - 0 * steps_per_color) / steps_per_color);
              factor2 = (float)((int)(ind) % num_steps) / steps_per_color;
              color = (rgb_t){
                .green=255 * factor1 + 0 * factor2, 
                .red=0 * factor1 + 255 * factor2, 
                .blue=0 * factor1 + 0 * factor2
              };
              spect_rgb_set_pixel(rgb, j, color);
              break;
      case 1: factor1 = 1.0 - ((float)(ind % num_steps - 1 * steps_per_color) / steps_per_color);
              factor2 = (float)((int)(ind - steps_per_color) % num_steps) / steps_per_color;
              color = (rgb_t){
                .green=0 * factor1 + 0 * factor2, 
                .red= 255 * factor1 + 0 * factor2, 
                .blue=0 * factor1 + 255 * factor2};
              spect_rgb_set_pixel(rgb, j, color);
              break;
      case 2: factor1 = 1.0 - ((float)(ind % num_steps - 2 * steps_per_color) / steps_per_color);
              factor2 = (float)((int)(ind - (2 * steps_per_color)) % num_steps) / steps_per_color;
              color = (rgb_t){
                .green=0 * factor1 + 255 * factor2, 
                .red=0 * factor1 + 0 * factor2, 
                .blue=255 * factor1 + 0 * factor2};
              spect_rgb_set_pixel(rgb, j, color);
              break;
    }
  }
  if(rainbow_state.effStep >= 30) {rainbow_state_reset(&rainbow_state, 30, rainbow_state.delay); return 0x03; }
  else rainbow_state.effStep++;
  return 0x01;
}
