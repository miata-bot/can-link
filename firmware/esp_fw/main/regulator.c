#include "regulator.h"

esp_err_t reg_initialize(reg_config_t* cfg, reg_t** out_ctx)
{
  gpio_set_direction(cfg->gpio_enable, GPIO_MODE_OUTPUT);
  // gpio_set_level(cfg->gpio_enable, !cfg->enable_value);

  reg_t* ctx = (reg_t*)malloc(sizeof(reg_t));
  if (!ctx) return ESP_ERR_NO_MEM;

  *ctx = (reg_t) {
    .cfg = *cfg,
  };

  *out_ctx = ctx;
  return ESP_OK;
}

void reg_deinit(reg_t* reg)
{
  (void)reg;
}

void reg_enable(reg_t* reg)
{
  // gpio_set_level(reg->cfg.gpio_enable, reg->cfg.enable_value);
  gpio_set_level(reg->cfg.gpio_enable, 1);
  reg->state = REG_ENABLED;
}

void reg_disable(reg_t* reg)
{
  // gpio_set_level(reg->cfg.gpio_enable, !reg->cfg.enable_value);
  gpio_set_level(reg->cfg.gpio_enable, 0);
  reg->state = REG_DISABLED;
}