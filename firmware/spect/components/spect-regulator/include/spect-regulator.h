#include <driver/gpio.h>

typedef enum SpectRegulatorState {
  SPECT_REGULATOR_OFF=0x0,
  SPECT_REGULATOR_ON=0x1,
} spect_regulator_state_t;

typedef struct SpectRegulatorConfig {
  gpio_num_t gpio;
} spect_regulator_cfg_t;

typedef struct SpectRegulator {
  spect_regulator_cfg_t* cfg;
  spect_regulator_state_t state;
} spect_regulator_t;

esp_err_t spect_regulator_init(spect_regulator_cfg_t* cfg, spect_regulator_t** out_ctx);
esp_err_t spect_regulator_enable(spect_regulator_t* ctx);
esp_err_t spect_regulator_disable(spect_regulator_t* ctx);