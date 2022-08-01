#ifndef SPECT_EFFECT_CHASE_H
#define SPECT_EFFECT_CHASE_H

#include "spect-rgb-channel.h"

void chase_init(spect_rgb_t* rgb, uint32_t on_color, uint32_t off_color);
void chase_loop(spect_rgb_t* rgb);

#endif