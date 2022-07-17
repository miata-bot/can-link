#ifndef SPECT_RADIO_H
#define SPECT_RADIO_H

#include "spect-config.h"
#include "sx1231.h"

esp_err_t spect_radio_initialize(spect_config_context_t* config_ctx, SX1231_config_t* cfg);
esp_err_t spect_radio_loop(spect_config_context_t* config_ctx);

#endif