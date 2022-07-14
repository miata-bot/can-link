#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "filesystem.h"

#include "spect-config.h"
#include "spect-ble.h"

static const char *TAG = "SPECT";

spect_config_context_t* config_ctx;

void app_main(void)
{
  ESP_LOGI(TAG, "app boot");
  ESP_LOGI(TAG, "free memory=%d", esp_get_minimum_free_heap_size());

  esp_err_t err;
  err = flash_init();
  ESP_ERROR_CHECK(err);
  
  ESP_LOGI(TAG, "mounted fs");

  spect_config_cfg_t cfg;
  cfg = (spect_config_cfg_t){
    .path="/flash/config.db",
  };
  err = spect_config_init(&cfg, &config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "config init ok");

  err = spect_config_load(config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "config load ok");

  ESP_LOGI(TAG, "loaded db");

  ESP_LOGI(TAG, "free memory=%d", esp_get_minimum_free_heap_size());
  ESP_LOGI(TAG, "mode=%d", config_ctx->config->state->mode);

  ESP_LOGI(TAG, "ble init");
  err = spect_ble_init(config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "ble init ok");

  ESP_LOGI(TAG, "mode=%s", spect_mode_to_string(config_ctx->config->state->mode));

  ESP_LOGI(TAG, "solid config, %d %d", config_ctx->config->state->data.solid.channel0, config_ctx->config->state->data.solid.channel1);
  // err = spect_set_state(config_ctx);
  // ESP_ERROR_CHECK(err);

  err = spect_set_mode(config_ctx, SPECT_MODE_EFFECT_RAINBOW);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "rainbow config, %d %d", config_ctx->config->state->data.solid.channel0, config_ctx->config->state->data.solid.channel1);
  
  while(true) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
