// /**
//  * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
//  *
//  * SPDX-License-Identifier: BSD-3-Clause
//  */

// #include <stdio.h>
// #include "pico/stdlib.h"

// int main() {
//     stdio_init_all();
//     while (true) {
//         printf("Hello, world!\n");
//         sleep_ms(1000);
//     }
//     return 0;
// }
#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define PIN_LED_R 13
#define PIN_LED_G 12
#define PIN_LED_B 11
#define WS2812_PIN 2

#define IS_RGBW false
#define NUM_PIXELS 150

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(urgb_u32(0, 0, 0xff));
        else
            put_pixel(0);
    }
}

void pattern_random(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    int max = 100; // let's not draw too much current!
    t %= max;
    for (int i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
};

int main() {
    // Tell the LED pin that the PWM is in charge of its value.
    gpio_set_function(PIN_LED_R, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED_G, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED_B, GPIO_FUNC_PWM);
    // Figure out which slice we just connected to the LED pin
    uint slice_num_r = pwm_gpio_to_slice_num(PIN_LED_R);
    uint slice_num_g = pwm_gpio_to_slice_num(PIN_LED_G);
    uint slice_num_b = pwm_gpio_to_slice_num(PIN_LED_B);

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

    pwm_set_gpio_level(PIN_LED_R, 0);
    pwm_set_gpio_level(PIN_LED_G, 0);
    pwm_set_gpio_level(PIN_LED_B, 0);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    int t = 0;

    while(true) {
        int pat = rand() % count_of(pattern_table);
        int dir = (rand() >> 30) & 1 ? 1 : -1;
        puts(pattern_table[pat].name);
        puts(dir == 1 ? "(forward)" : "(backward)");
        for (int i = 0; i < 1000; ++i) {
            pattern_table[pat].pat(NUM_PIXELS, t);
            sleep_ms(10);
            t += dir;
        }


    for(int i = 0; i < 255; i++) {
        pwm_set_gpio_level(PIN_LED_R, 255 * i);
        sleep_ms(10);
    }
    
    for(int i = 255; i >= 0; i--) {
        pwm_set_gpio_level(PIN_LED_R, 255 * i);
        sleep_ms(10);
    }

    // for(int i = 0; i < 255; i++) {
    //     pwm_set_gpio_level(PIN_LED_G, 255 * i);
    //     sleep_ms(10);
    // }
    
    // for(int i = 255; i >= 0; i--) {
    //     pwm_set_gpio_level(PIN_LED_G, 255 * i);
    //     sleep_ms(10);
    // }

    for(int i = 0; i < 255; i++) {
        pwm_set_gpio_level(PIN_LED_B, 255 * i);
        sleep_ms(10);
    }
    
    for(int i = 255; i >= 0; i--) {
        pwm_set_gpio_level(PIN_LED_B, 255 * i);
        sleep_ms(10);
    }
    }
}