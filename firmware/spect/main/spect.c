#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "driver/spi_master.h"

#include "pins.h"

#include "filesystem.h"
#include "spect-config.h"
#include "spect-ble.h"

#include "spect-rgb-channel.h"
#include "rainbow.h"

#include "spect-radio.h"

static const char *TAG = "SPECT";

spect_config_context_t* config_ctx;
spect_rgb_t* channel0;

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
  
  /* start the watchdog task. It's possible this bad boy should probably
   * be after the initialization of everything?
   */ 
  static uint8_t ucParameterToPass;
  TaskHandle_t xHandle = NULL;

  /* Create the watchdog task, storing the handle.  Note that the passed parameter ucParameterToPass
   * must exist for the lifetime of the task, so in this case is declared static.  If it was just an
   * an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
   * the new task attempts to access it.
  */
  xTaskCreate(watchdog_task, "WATCHDOG", 1024, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);
  configASSERT(xHandle);

  esp_err_t err;

  ESP_LOGI(TAG, "SPI init");
  spi_bus_config_t buscfg = {
      .miso_io_num = GPIO_NUM_SPI2_MISO,
      .mosi_io_num = GPIO_NUM_SPI2_MOSI,
      .sclk_io_num = GPIO_NUM_SPI2_SCLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };

  // Initialize the SPI bus in HSPI mode. DMA channel might need changing later?
  err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "SPI init ok");

  /* flash partition is formatted as fat32. 
   * It is NOT encrypted. sorry. if someone really wants this
   * i'll make it, i just don't want too.
  */
  err = flash_init();
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "mounted fs");

  /* opens the database, but does not do any database operations */
  spect_config_cfg_t cfg;
  cfg = (spect_config_cfg_t){
    .path="/flash/config.db",
  };

  err = spect_config_init(&cfg, &config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "config init ok");

  /* loads config from sqlite database. */
  err = spect_config_load(config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "config load ok");

  ESP_LOGI(TAG, "loaded db");
  ESP_LOGI(TAG, "free memory=%d", esp_get_minimum_free_heap_size());
  ESP_LOGI(TAG, "mode=%d", config_ctx->config->state->mode);

  /* spawns a thread, consider putting this on a different core. */
  ESP_LOGI(TAG, "ble init");
  err = spect_ble_init(config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "ble init ok");

  /* 
   * very important variable. controls program flow. 
   * it is marked static because it should be treated as 
   * the PRIMARY mode switch. this allows this address to be
   * not thread safe. mode switching should be handled by
   * setting the global config_ctx mode, this handles all
   * cleanup
   */
  spect_mode_t current_mode = config_ctx->config->state->mode;
  ESP_LOGI(TAG, "bootup mode=%s", spect_mode_to_string(current_mode));

  /* begin initialization of peripherals */

  spect_rgb_config_t rgb_cfg;
  rgb_cfg = (spect_rgb_config_t) {
    .num_leds=config_ctx->config->strip_channel_1_length,
    .led_strip_gpio=GPIO_NUM_STRIP0,
    .ledc_channel_offset=0,
    .led_red_gpio=GPIO_NUM_RGB0_B,
    .led_green_gpio=GPIO_NUM_RGB0_G,
    .led_blue_gpio=GPIO_NUM_RGB0_R,
  };
  err = spect_rgb_install();
  ESP_ERROR_CHECK(err);
  // TODO: implement second channel

  /* rgb initializing */
  err = spect_rgb_initialize(&rgb_cfg, &channel0);
  ESP_ERROR_CHECK(err);
  err = spect_rgb_enable_strip(channel0);
  ESP_ERROR_CHECK(err);

  rgb_t color = {.red=0, .green=0, .blue=0};
  spect_rgb_fill(channel0, 0, channel0->strip->length, color);
  spect_rgb_blit(channel0);
  spect_rgb_wait(channel0);

  SX1231_config_t radio_cfg = {
      .gpio_cs = GPIO_NUM_SX1231_CSN,
      .gpio_int = GPIO_NUM_SX1231_IRQ,
      .gpio_reset = PIN_NUM_SX1231_RESET,
      .freqBand = RF69_915MHZ,
      .nodeID = 100, 
      .networkID = 100,
      .isRFM69HW_HCW = true,
      .host = SPI2_HOST
  };

/* VERY IMPORTANT!!!!!! DO NOT CREATE MORE STACK VARIABLES HERE U FOOL!!! */
main_loop_init:
  if(current_mode == SPECT_MODE_EFFECT_RAINBOW)  {
    ESP_LOGI(TAG, "Starting rainbow");
    rainbow_state_init(channel0, 
                       channel0->strip->length, 
                       config_ctx->config->state->data.rainbow.delay_time);
  }

  if(current_mode == SPECT_MODE_EFFECT_SOLID) {
    uint8_t a[4] = {0}; // FIX THIS!!!
    a[0] = config_ctx->config->state->data.solid.channel0;
    a[1] = config_ctx->config->state->data.solid.channel0 >>  8;
    a[2] = config_ctx->config->state->data.solid.channel0 >> 16;
    a[3] = config_ctx->config->state->data.solid.channel0 >> 24;
    rgb_t color_; // FIX THIS!!

    color_.red = a[1];
    color_.green = a[0];
    color_.blue = a[2];
    ESP_LOGE("LED", "fill %02X %02X %02X", color_.red, color_.green, color_.blue);
    led_strip_fill(config_ctx->rgb0->strip, 0, config_ctx->rgb0->strip->length, color_);
    led_strip_wait(config_ctx->rgb0->strip, 1000);
    led_strip_flush(config_ctx->rgb0->strip);
  }

  if(current_mode == SPECT_MODE_EFFECT_PULSE) {
    ESP_LOGE("LED", "SPECT_MODE_EFFECT_PULSE not implemented");
    abort();
  }

  if(current_mode == SPECT_MODE_RADIO) {
    ESP_LOGI("RADIO", "SPECT_MODE_RADIO init");
    spect_radio_initialize(config_ctx, &radio_cfg);
  }

  /* Main loop begins here. */
  while(config_ctx->config->state->mode == current_mode) {
    /* each mode should implement a `loop` function where 
     * relevant. This `loop` should only be one step of the loop
     * not enter an endless loop. This is subtly quite important
     * the main control loop will jump back before this point
     * all control flow is handled in this way, meaning if 
     * a mode takes too long to "step" it's state, the entire
     * UX will suffer */
    if(current_mode == SPECT_MODE_EFFECT_SOLID) {};
    if(current_mode == SPECT_MODE_EFFECT_RAINBOW) rainbow_loop(channel0);
    if(current_mode == SPECT_MODE_EFFECT_PULSE) {}
    if(current_mode == SPECT_MODE_RADIO) spect_radio_loop(config_ctx);
    esp_task_wdt_reset();
  }
  ESP_LOGW(TAG, "mode changed from %s to %s", spect_mode_to_string(current_mode), spect_mode_to_string(config_ctx->config->state->mode));
  /* do not reset any other state here. all initialization should 
   * be done at the next address */
  current_mode = config_ctx->config->state->mode;
  goto main_loop_init;
}
