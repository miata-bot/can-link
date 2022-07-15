#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"

#include "pins.h"

#include "filesystem.h"
#include "spect-config.h"
#include "spect-ble.h"
#include "spect-rgb-channel.h"
#include "rainbow.h"

static const char *TAG = "SPECT";

spect_config_context_t* config_ctx;
spect_rgb_t* channel0;
// spect_rgb_t* channel1;

void watchdog_task(void* params)
{
  while(true) {
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

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

  spect_mode_t current_mode = config_ctx->config->state->mode;
  ESP_LOGI(TAG, "mode=%s", spect_mode_to_string(current_mode));

  // ESP_LOGI(TAG, "solid config, %d %d", config_ctx->config->state->data.solid.channel0, config_ctx->config->state->data.solid.channel1);
  // err = spect_set_state(config_ctx);
  // ESP_ERROR_CHECK(err);

  // err = spect_set_mode(config_ctx, SPECT_MODE_EFFECT_RAINBOW);
  // ESP_ERROR_CHECK(err);
  // ESP_LOGI(TAG, "rainbow config, %d %d", config_ctx->config->state->data.solid.channel0, config_ctx->config->state->data.solid.channel1);
  spect_rgb_config_t rgb_cfg;
  rgb_cfg = (spect_rgb_config_t) {
    .num_leds=30,
    .led_strip_gpio=GPIO_NUM_STRIP0,
    .ledc_channel_offset=0,
    .led_red_gpio=GPIO_NUM_RGB0_B,
    .led_green_gpio=GPIO_NUM_RGB0_G,
    .led_blue_gpio=GPIO_NUM_RGB0_R,
  };
  err = spect_rgb_install();
  ESP_ERROR_CHECK(err);

  err = spect_rgb_initialize(&rgb_cfg, &channel0);
  ESP_ERROR_CHECK(err);
  err = spect_rgb_enable_strip(channel0);
  ESP_ERROR_CHECK(err);
  config_ctx->rgb0 = channel0;

  rgb_t color = {.red=0, .green=0, .blue=0};
  spect_rgb_fill(channel0, 0, channel0->strip->length, color);
  spect_rgb_blit(channel0);
  spect_rgb_wait(channel0);

  static uint8_t ucParameterToPass;
  TaskHandle_t xHandle = NULL;

  // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
  // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
  // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
  // the new task attempts to access it.
  xTaskCreate( watchdog_task, "NAME", 1024, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
  configASSERT( xHandle );

  // ESP_LOGI(TAG, "Starting rainbow");
  // strip_state_init(1);

  // while(true) {
  //   strips_loop(channel0->strip);
  //   vTaskDelay(0);
  // }

main_loop:
  if(current_mode == SPECT_MODE_EFFECT_RAINBOW)  {
    ESP_LOGI(TAG, "Starting rainbow");
    strip_state_init(1);
  }
  if(current_mode == SPECT_MODE_EFFECT_SOLID) {
    uint8_t a[4] = {0};
    a[0] = config_ctx->config->state->data.solid.channel0;
    a[1] = config_ctx->config->state->data.solid.channel0 >>  8;
    a[2] = config_ctx->config->state->data.solid.channel0 >> 16;
    a[3] = config_ctx->config->state->data.solid.channel0 >> 24;
    rgb_t color_;

    color_.red = a[1];
    color_.green = a[0];
    color_.blue = a[2];
    ESP_LOGE("LED", "fill %02X %02X %02X", color_.red, color_.green, color_.blue);
    led_strip_fill(config_ctx->rgb0->strip, 0, config_ctx->rgb0->strip->length, color_);
    led_strip_wait(config_ctx->rgb0->strip, 1000);
    led_strip_flush(config_ctx->rgb0->strip);
  }


  while(config_ctx->config->state->mode == current_mode) {
    if(current_mode == SPECT_MODE_EFFECT_RAINBOW) 
      strips_loop(channel0->strip);

    if(current_mode == SPECT_MODE_EFFECT_SOLID) {}

    vTaskDelay(1);
  }
  ESP_LOGW(TAG, "mode changed from %s to %s", spect_mode_to_string(current_mode), spect_mode_to_string(config_ctx->config->state->mode));
  current_mode = config_ctx->config->state->mode;
  goto main_loop;
}
