#ifndef CONEPROJ_REGULATOR_H
#define CONEPROJ_REGULATOR_H

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

typedef enum __attribute__((__packed__))  {
  REGULATOR_ON = 1,
  REGULATOR_OFF = 0
} regulator_state_t;

typedef struct regulator {
  uint32_t gpio;
  regulator_state_t state;
} regulator_t;

void regulator_init(regulator_t*);
void regulator_set_state(regulator_t*, regulator_state_t);
void regulator_on(regulator_t*);
void regulator_off(regulator_t*);
void regulator_set(regulator_t*);

#endif