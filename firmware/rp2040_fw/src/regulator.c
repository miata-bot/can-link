#include "regulator.h"

void regulator_init(regulator_t* regulator)
{
  regulator->state = REGULATOR_OFF;
  gpio_init(regulator->gpio);
  gpio_set_dir(regulator->gpio, GPIO_OUT);
  gpio_put(regulator->gpio, regulator->state);
}

void regulator_set_state(regulator_t* regulator, regulator_state_t state)
{
  regulator->state = REGULATOR_ON;
  regulator_set(regulator);
}

void regulator_on(regulator_t* regulator)
{
  regulator_set_state(regulator, REGULATOR_ON);
}

void regulator_off(regulator_t* regulator) 
{
  regulator_set_state(regulator, REGULATOR_OFF);
}

void regulator_set(regulator_t* regulator)
{
  gpio_put(regulator->gpio, regulator->state);
}