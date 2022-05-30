#ifndef CONEPROJ_REGULATOR_H
#define CONEPROJ_REGULATOR_H

#include <stdint.h>

#include <esp_log.h>
#include <esp_system.h>
#include <driver/gpio.h>

typedef enum {
  REG_DISABLED = 0,
  REG_ENABLED  = 1
} reg_state_t;

typedef enum {
  REG_HIGH = 1,
  REG_LOW  = 0
} reg_enable_value_t;

typedef struct {
  gpio_num_t          gpio_enable;
  reg_enable_value_t  enable_value;
} reg_config_t;

typedef struct reg {
  reg_config_t cfg;
  reg_state_t state;
} reg_t;

esp_err_t reg_initialize(reg_config_t*, reg_t**);
void reg_deinit(reg_t*);

void reg_enable(reg_t*);
void reg_disable(reg_t*);

#endif