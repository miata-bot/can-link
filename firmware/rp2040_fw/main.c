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
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define PIN_LED_R 28
#define PIN_LED_G 27
#define PIN_LED_B 26
#define WS2812_PIN 4
#define STATUS_LED_GREEN_PIN 16
#define STATUS_LED_RED_PIN 17
#define REG_EN 5

#ifdef PICO_DEFAULT_SPI_SCK_PIN
#undef PICO_DEFAULT_SPI_SCK_PIN
#endif

#ifdef PICO_DEFAULT_SPI_TX_PIN
#undef PICO_DEFAULT_SPI_TX_PIN
#endif

#ifdef PICO_DEFAULT_SPI_RX_PIN
#undef PICO_DEFAULT_SPI_RX_PIN
#endif

#ifdef PICO_DEFAULT_SPI_CSN_PIN
#undef PICO_DEFAULT_SPI_CSN_PIN
#endif

#define PICO_DEFAULT_SPI_RX_PIN  0
#define PICO_DEFAULT_SPI_TX_PIN  3
#define PICO_DEFAULT_SPI_SCK_PIN 2
#define PICO_DEFAULT_SPI_CSN_PIN  1

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

#define BUF_LEN         1

void printbuf(uint8_t buf[], size_t len) {
    int i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}

void spi_test() {
    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000*1000);
    spi_set_slave(spi_default, true);
    spi_set_format(spi_default, 8, SPI_CPHA_0, SPI_CPOL_0, SPI_MSB_FIRST);

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    uint8_t out_buf[BUF_LEN], in_buf[BUF_LEN];

    // Initialize output buffer
    for (size_t i = 0; i < BUF_LEN; ++i) {
        out_buf[i] = i;
    }

    printf("SPI slave says: When reading from MOSI, the following buffer will be written to MISO:\n");
    printbuf(out_buf, BUF_LEN);

    for (size_t i = 0; ; ++i) {
        // printf("waiting CSN HIGH\n");
        // while(gpio_get(PICO_DEFAULT_SPI_CSN_PIN) == 1) {}

        printf("starting xaction\n");

        // // Write the output buffer to MISO, and at the same time read from MOSI.
        // spi_write_read_blocking(spi_default, out_buf, in_buf, BUF_LEN);
        // // pio_spi_write8_read8_blocking(&spi, out_buf, in_buf, BUF_LEN);
        spi_read_blocking(spi_default, 0x69, in_buf, BUF_LEN);

        // // Write to stdio whatever came in on the MOSI line.
        printf("SPI slave says: read page %d from the MOSI line:\n", i);
        printbuf(in_buf, BUF_LEN);
        // printf("waiting CSN LOW\n");
        // while(gpio_get(PICO_DEFAULT_SPI_CSN_PIN) == 0) {}
        printf("xaction done\n\n");
    }
}

void blink_status_led_red() {
        gpio_put(STATUS_LED_GREEN_PIN, 1);
        sleep_ms(250);
        gpio_put(STATUS_LED_GREEN_PIN, 0);
        sleep_ms(250);
}

int main() {
    stdio_init_all();

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    for(int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(0);
    }

    while (!stdio_usb_connected()) {
        tight_loop_contents();
    }

    gpio_init(REG_EN);
    gpio_set_dir(REG_EN, GPIO_OUT);
    gpio_put(REG_EN, 0);

    gpio_init(STATUS_LED_GREEN_PIN);
    gpio_set_dir(STATUS_LED_GREEN_PIN, GPIO_OUT);
    gpio_put(STATUS_LED_GREEN_PIN, 1);

    gpio_init(STATUS_LED_RED_PIN);
    gpio_set_dir(STATUS_LED_RED_PIN, GPIO_OUT);
    gpio_put(STATUS_LED_RED_PIN, 1);

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



    int t = 0;

    while(true) {

        gpio_put(STATUS_LED_RED_PIN, 1);
        sleep_ms(250);
        gpio_put(STATUS_LED_RED_PIN, 0);
        sleep_ms(250);
        gpio_put(REG_EN, 1);

        int pat = rand() % count_of(pattern_table);
        int dir = (rand() >> 30) & 1 ? 1 : -1;
        puts(pattern_table[pat].name);
        puts(dir == 1 ? "(forward)" : "(backward)");
        for (int i = 0; i < 1000; ++i) {
            pattern_table[pat].pat(NUM_PIXELS, t);
            sleep_ms(10);
            t += dir;
        }

        for(int i = 0; i < NUM_PIXELS; i++) {
            put_pixel(0);
        }

        blink_status_led_red();

        for(int i = 0; i < 255; i++) {
            pwm_set_gpio_level(PIN_LED_R, 255 * i);
            sleep_ms(10);
        }

        blink_status_led_red();
        
        for(int i = 255; i >= 0; i--) {
            pwm_set_gpio_level(PIN_LED_R, 255 * i);
            sleep_ms(10);
        }

        blink_status_led_red();

        for(int i = 0; i < 255; i++) {
            pwm_set_gpio_level(PIN_LED_G, 255 * i);
            sleep_ms(10);
        }

        blink_status_led_red();
    
        for(int i = 255; i >= 0; i--) {
            pwm_set_gpio_level(PIN_LED_G, 255 * i);
            sleep_ms(10);
        }

        blink_status_led_red();

        for(int i = 0; i < 255; i++) {
            pwm_set_gpio_level(PIN_LED_B, 255 * i);
            sleep_ms(10);
        }

        blink_status_led_red();
        
        for(int i = 255; i >= 0; i--) {
            pwm_set_gpio_level(PIN_LED_B, 255 * i);
            sleep_ms(10);
        }
        blink_status_led_red();

    }
}