#ifndef SPECT_EFFECT_PULSE_H
#define SPECT_EFFECT_PULSE_H

#include "spect-rgb-channel.h"

void pulse_init(spect_rgb_t* rgb, uint32_t on_color, uint32_t off_color);
void pulse_loop(spect_rgb_t* rgb);
#endif