#include "status_led.h"

void status_led_init(status_led_t* status_led)
{
  gpio_init(status_led->gpio);
  gpio_set_dir(status_led->gpio, GPIO_OUT);
  gpio_put(status_led->gpio, 1);
  status_led->state = STATUS_LOW;
}

void status_led_blink(status_led_t* status_led, uint32_t blink_time)
{
  status_led_set_state(status_led, STATUS_LOW);
  status_led_set_state(status_led, STATUS_HIGH);
  sleep_ms(blink_time);
  status_led_set_state(status_led, STATUS_LOW);
}

void status_led_set_state(status_led_t* status_led, status_led_state_t state)
{
  switch(state) {
    case STATUS_HIGH:
      gpio_put(status_led->gpio, 0);
    break;
    case STATUS_LOW:
      gpio_put(status_led->gpio, 1);
    break;
  }
  status_led->state = state;
}
