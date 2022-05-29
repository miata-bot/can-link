#include "rgb.h"

void rgb_init(RGB_t* rgb)
{

  // Tell the LED pin that the PWM is in charge of its value.
  gpio_set_function(rgb->gpio_r, GPIO_FUNC_PWM);
  gpio_set_function(rgb->gpio_g, GPIO_FUNC_PWM);
  gpio_set_function(rgb->gpio_b, GPIO_FUNC_PWM);

  // Figure out which slice we just connected to the LED pin
  uint slice_num_r = pwm_gpio_to_slice_num(rgb->gpio_r);
  uint slice_num_g = pwm_gpio_to_slice_num(rgb->gpio_g);
  uint slice_num_b = pwm_gpio_to_slice_num(rgb->gpio_b);

  // Get some sensible defaults for the slice configuration. By default, the
  // counter is allowed to wrap over its maximum range (0 to 2**16-1)
  pwm_config config_r = pwm_get_default_config();
  pwm_config config_g = pwm_get_default_config();
  pwm_config config_b = pwm_get_default_config();

  // Set divider, reduces counter clock to sysclock/this value
  pwm_config_set_clkdiv(&config_r, 4.f);
  pwm_config_set_clkdiv(&config_g, 4.f);
  pwm_config_set_clkdiv(&config_b, 4.f);

  // Load the configuration into our PWM slice, and set it running.
  pwm_init(slice_num_r, &config_r, true);
  pwm_init(slice_num_g, &config_g, true);
  pwm_init(slice_num_b, &config_b, true);

  pwm_set_gpio_level(rgb->gpio_r, 0);
  pwm_set_gpio_level(rgb->gpio_g, 0);
  pwm_set_gpio_level(rgb->gpio_b, 0);
  rgb_disable(rgb);
}

void rgb_enable(RGB_t* rgb)
{
  rgb->r = 255; rgb->g = 255; rgb->b = 255;
  rgb->brightness = 255;
  rgb_set(rgb);
}

void rgb_disable(RGB_t* rgb)
{
  rgb->r = 0; rgb->g = 0; rgb->b = 0;
  rgb->brightness = 0;
  rgb_set(rgb);
}

void rgb_set_color(RGB_t* rgb, uint8_t r, uint8_t g, uint8_t b)
{
  rgb->r = r; rgb->g = g; rgb->b = b;
  rgb_set(rgb);
}

void rgb_set_brightness(RGB_t* rgb, uint8_t brightness)
{
  rgb->brightness = brightness;
  rgb_set(rgb);
}

void rgb_set(RGB_t* rgb) {
  pwm_set_gpio_level(rgb->gpio_r, rgb->r * rgb->brightness);
  pwm_set_gpio_level(rgb->gpio_g, rgb->g * rgb->brightness);
  pwm_set_gpio_level(rgb->gpio_b, rgb->b * rgb->brightness);
}