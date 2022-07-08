#include <led_strip.h>
extern led_strip_t strip;

typedef struct StripState {uint8_t effect; uint8_t effects; uint16_t effStep; unsigned long effStart;} strip_state_t;
typedef struct StripLoop { uint8_t currentChild; uint8_t childs; bool timeBased; uint16_t cycles; uint16_t currentTime;} strip_loop_t;
strip_state_t strip_0;
strip_loop_t strip0loop0;

void strip_state_loop_init(strip_loop_t* loop, uint8_t totchilds, bool timebased, uint16_t tottime) {
  loop->currentTime=0;
  loop->currentChild=0;
  loop->childs=totchilds;
  loop->timeBased=timebased;
  loop->cycles=tottime;
}

void strip_state_reset(strip_state_t* strip_state)
{
    strip_state->effStep = 0;
    strip_state->effect = (strip_state->effect + 1) % strip_state->effects;
    strip_state->effStart = esp_timer_get_time();
}

void strip_state_init(uint8_t toteffects)
{
  strip_0.effect = -1;
  strip_0.effects = toteffects;
  strip_state_loop_init(&strip0loop0, 1, false, 1);
  strip_state_reset(&strip_0);
}

uint8_t strip0_loop0_eff0();
uint8_t strip0_loop0();

void strips_loop() {
  if(strip0_loop0() & 0x01) {
    led_strip_flush(&strip);
    led_strip_wait(&strip, 1000);
  }
}

uint8_t strip0_loop0() {
  uint8_t ret = 0x00;
  switch(strip0loop0.currentChild) {
    case 0: 
           ret = strip0_loop0_eff0();break;
  }
  if(ret & 0x02) {
    ret &= 0xfd;
    if(strip0loop0.currentChild + 1 >= strip0loop0.childs) {
      strip0loop0.currentChild = 0;
      if(++strip0loop0.currentTime >= strip0loop0.cycles) {strip0loop0.currentTime = 0; ret |= 0x02;}
    }
    else {
      strip0loop0.currentChild++;
    }
  };
  return ret;
}

uint8_t strip0_loop0_eff0() {
    // Strip ID: 0 - Effect: Rainbow - LEDS: 120
    // Steps: 122 - Delay: 20
    // Colors: 3 (255.0.0, 0.255.0, 0.0.255)
    // Options: rainbowlen=120, toLeft=true, 
  if(esp_timer_get_time() - strip_0.effStart < 20 * (strip_0.effStep)) return 0x00;
  float factor1, factor2;
  uint16_t ind;
  rgb_t color;
  for(uint16_t j=0;j<120;j++) {
    ind = strip_0.effStep + j * 1.0166666666666666;
    switch((int)((ind % 122) / 40.666666666666664)) {
      case 0: factor1 = 1.0 - ((float)(ind % 122 - 0 * 40.666666666666664) / 40.666666666666664);
              factor2 = (float)((int)(ind - 0) % 122) / 40.666666666666664;
              color = (rgb_t){.green=255 * factor1 + 0 * factor2, .red=0 * factor1 + 255 * factor2, .blue=0 * factor1 + 0 * factor2};
              led_strip_set_pixel(&strip, j, color);
              break;
      case 1: factor1 = 1.0 - ((float)(ind % 122 - 1 * 40.666666666666664) / 40.666666666666664);
              factor2 = (float)((int)(ind - 40.666666666666664) % 122) / 40.666666666666664;
              color = (rgb_t){.green=0 * factor1 + 0 * factor2, .red= 255 * factor1 + 0 * factor2, .blue=0 * factor1 + 255 * factor2};
              led_strip_set_pixel(&strip, j, color);
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 122 - 2 * 40.666666666666664) / 40.666666666666664);
              factor2 = (float)((int)(ind - 81.33333333333333) % 122) / 40.666666666666664;
              color = (rgb_t){.green=0 * factor1 + 255 * factor2, .red=0 * factor1 + 0 * factor2, .blue=255 * factor1 + 0 * factor2};
              led_strip_set_pixel(&strip, j, color);
              break;
    }
  }
  if(strip_0.effStep >= 122) {strip_state_reset(&strip_0); return 0x03; }
  else strip_0.effStep++;
  return 0x01;
}

