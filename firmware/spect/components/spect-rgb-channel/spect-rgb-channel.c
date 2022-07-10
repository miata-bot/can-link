#include <stdio.h>
#include "spect-rgb-channel.h"

/*
 * This callback function will be called when fade operation has ended
 * Use callback only if you are aware it is being called inside an ISR
 * Otherwise, you can use a semaphore to unblock tasks
 */
static bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
  portBASE_TYPE taskAwoken = pdFALSE;
  spect_rgb_t* ctx = (spect_rgb_t*)usr_arg;

  if (param->event == LEDC_FADE_END_EVT) {
    xSemaphoreGiveFromISR(ctx->counting_sem, &taskAwoken);
  }

  return (taskAwoken == pdTRUE);
}

esp_err_t spect_rgb_install()
{
  /*
  * Prepare and set configuration of timers
  * that will be used by LED Controller
  */
  ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_8_BIT,  // resolution of PWM duty
    .freq_hz = 5000,                      // frequency of PWM signal
    .speed_mode = LEDC_LOW_SPEED_MODE,           // timer mode
    .timer_num = LEDC_LS_TIMER,            // timer index
    .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
  };
  // Set configuration of timer0 for high speed channels
  ledc_timer_config(&ledc_timer);
  led_strip_install();
  ledc_fade_func_install(0);
  return ESP_OK;
}

esp_err_t spect_rgb_initialize(spect_rgb_config_t* config, spect_rgb_t** out_ctx)
{
  esp_err_t err = ESP_OK;

  led_strip_t strip = {
    .type = LED_STRIP_WS2812,
    .length = config->num_leds,
    .gpio = config->led_strip_gpio,
    .buf = NULL,
    // .brightness = 255,
  };
  err = led_strip_init(&strip);
  if(err != ESP_OK)
    return err;

  spect_rgb_t* ctx = (spect_rgb_t*)malloc(sizeof(spect_rgb_t));
  if (!ctx) return ESP_ERR_NO_MEM;
  *ctx = (spect_rgb_t){
    .cfg = config,
    .ledc_channels = {
      {
        .channel    = ledc_channel_offset,
        .duty       = 0,
        .gpio_num   = config->led_red_channel,
        .speed_mode = LEDC_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_LS_TIMER,
        .flags.output_invert = 0
      },
      {
        .channel    = ledc_channel_offset+1,
        .duty       = 0,
        .gpio_num   = config->led_green_channel,
        .speed_mode = LEDC_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_LS_TIMER,
        .flags.output_invert = 0
      },
      {
        .channel    = ledc_channel_offset+2,
        .duty       = 0,
        .gpio_num   = config->led_blue_channel,
        .speed_mode = LEDC_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_LS_TIMER,
        .flags.output_invert = 0
      }
    }
  }
  ctx->counting_sem = xSemaphoreCreateCounting(3, 0);

  ledc_channel_config(&ctx->ledc_channels[LEDC_CHANNEL_R]);
  ledc_channel_config(&ctx->ledc_channels[LEDC_CHANNEL_G]);
  ledc_channel_config(&ctx->ledc_channels[LEDC_CHANNEL_B]);

  ledc_cbs_t callbacks = {
    .fade_cb = cb_ledc_fade_end_event
  };
  ledc_cb_register(ctx->ledc_channels[LEDC_CHANNEL_R].speed_mode, ctx->ledc_channels[LEDC_CHANNEL_R].channel, &callbacks, (void *)ctx);
  ledc_cb_register(ctx->ledc_channels[LEDC_CHANNEL_G].speed_mode, ctx->ledc_channels[LEDC_CHANNEL_B].channel, &callbacks, (void *)ctx);
  ledc_cb_register(ctx->ledc_channels[LEDC_CHANNEL_B].speed_mode, ctx->ledc_channels[LEDC_CHANNEL_B].channel, &callbacks, (void *)ctx);

  ledc_stop(ctx->ledc_channels[LEDC_CHANNEL_R].speed_mode, ctx->ledc_channels[LEDC_CHANNEL_R].channel, 1);
  ledc_stop(ctx->ledc_channels[LEDC_CHANNEL_G].speed_mode, ctx->ledc_channels[LEDC_CHANNEL_G].channel, 1);
  ledc_stop(ctx->ledc_channels[LEDC_CHANNEL_B].speed_mode, ctx->ledc_channels[LEDC_CHANNEL_B].channel, 1);

  *out_ctx = ctx;
  return ESP_OK;
}

esp_err_t spect_rgb_enable_pwm(spect_rgb_t* spect_rgb){ return ESP_OK; }

esp_err_t spect_rgb_disable_pwm(spect_rgb_t* spect_rgb){ return ESP_OK; }

esp_err_t spect_rgb_enable_strip(spect_rgb_t* spect_rgb){ return ESP_OK; }

esp_err_t spect_rgb_disable_strip(spect_rgb_t* spect_rgb){ return ESP_OK; }

esp_err_t spect_rgb_set_color(spect_rgb_t* spect_rgb, rgb_t color)
{
  esp_err_t err = ledc_set_duty(ctx->ledc_channels[LEDC_CHANNEL_R].speed_mode, ledc_channel[LEDC_CHANNEL_R].channel, color.r);
  return err;
}

esp_err_t spect_rgb_set_pixel(spect_rgb_t* spect_rgb, uint16_t address, rgb_t color)
{
  esp_err_t err = led_strip_set_pixel(spect_rgb->strip, address, color);
  return err;
}

esp_err_t spect_rgb_fill(spect_rgb_t* spect_rgb, uint16_t start, uint16_t end, rgb_t color)
{
  esp_err_t err = led_strip_fill(spect_rgb->strip, start, end, color);
  return err;
}

esp_err_t spect_rgb_blit(spect_rgb_t* spect_rgb)
{
  esp_err_t err = led_strip_flush(spect_rgb->strip);
  return err;
}

esp_err_t spect_rgb_wait(spect_rgb_t* ctx)
{
  esp_err_t err = led_strip_wait(ctx, portMAX_DELAY);
  if(err != ESP_OK)
    return err;

  err = ledc_update_duty(strip->ledc_channels[LEDC_CHANNEL_R].speed_mode, strip->ledc_channels[LEDC_CHANNEL_R].channel);
  if(err != ESP_OK)
    return err;
  
  err = ledc_update_duty(strip->ledc_channels[LEDC_CHANNEL_G].speed_mode, strip->ledc_channels[LEDC_CHANNEL_G].channel);
  if(err != ESP_OK)
    return err;

  err = ledc_update_duty(strip->ledc_channels[LEDC_CHANNEL_B].speed_mode, strip->ledc_channels[LEDC_CHANNEL_B].channel);
  return err;
}
