#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "filesystem.h"

#include "spect-config.h"
// #include <spect-rgb-channel.h>
// #include <spect-regulator.h>

static const char *TAG = "SPECT";

spect_config_context_t* config;

// spect_regulator_t reg0;
// spect_regulator_t reg1;

// spect_rgb_t rgb0;
// spect_rgb_t rgb1;

void app_main(void)
{
  ESP_LOGI(TAG, "app boot");

  esp_err_t err;
  err = flash_init();
  ESP_ERROR_CHECK(err);
  
  ESP_LOGI(TAG, "mounted fs");

  spect_config_cfg_t cfg;
  cfg = (spect_config_cfg_t){
    .path="/flash/config.db",
  };
  err = spect_config_init(&cfg, &config);
  ESP_ERROR_CHECK(err);

  err = spect_config_load(config);
  ESP_ERROR_CHECK(err);

  ESP_LOGI(TAG, "loaded db");
  while(true) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
