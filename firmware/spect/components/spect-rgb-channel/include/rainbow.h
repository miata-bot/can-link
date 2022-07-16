#ifndef RAINBOW_H
#define RAINBOW_H

#include "spect-rgb-channel.h"

typedef struct StripState {uint8_t effect; uint8_t effects; uint16_t effStep; unsigned long effStart; uint16_t numSteps; uint16_t delay;} rainbow_state_t;
typedef struct StripLoop  {uint8_t currentChild; uint8_t childs; bool timeBased; uint16_t cycles; uint16_t currentTime;} rainbow_loop_t;

void rainbow_state_init(spect_rgb_t* rgb, uint16_t num_steps, uint16_t delay);
void rainbow_loop(spect_rgb_t* rgb);

// private

uint8_t rainbow_step(spect_rgb_t* rgb);
uint8_t rainbow_step_loop(spect_rgb_t* rgb);
void rainbow_loop_init(rainbow_loop_t* loop, uint8_t totchilds, bool timebased, uint16_t tottime);
void rainbow_state_reset(rainbow_state_t* rainbow_state, uint16_t num_steps, uint16_t delay);
#endif