// /**
//  * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
//  *
//  * SPDX-License-Identifier: BSD-3-Clause
//  */
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

#include "logging.h"
#include "command.h"
#include "regulator.h"
#include "rgb.h"
#include "status_led.h"
#include "ws2812.h"

#define RGB0_R_PIN 28
#define RGB0_G_PIN 27
#define RGB0_B_PIN 26

#define RGB1_R_PIN 25
#define RGB1_G_PIN 24
#define RGB1_B_PIN 23

#define WS2812_PIN 4

#define STATUS_LED_GREEN_PIN 16
#define STATUS_LED_RED_PIN 17
#define REG_EN_PIN 5

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

int main() {
    stdio_init_all();

    // wait for USB
    while (!stdio_usb_connected())
        tight_loop_contents();

    PICO_LOGI("initialize status LEDs");

    status_led_t status_led_green = {
        .gpio = STATUS_LED_GREEN_PIN
    };
    status_led_init(&status_led_green);

    status_led_t status_led_red = {
        .gpio = STATUS_LED_RED_PIN
    };
    status_led_init(&status_led_red);

    PICO_LOGI("initialize status LEDs OK");

    PICO_LOGI("initialize ws2812");

    ws2812_t ws2812 = {
        .pio = pio0,
        .sm = 0,
        .offset = 0,
        .gpio = WS2812_PIN,
        .is_rgbw = true,
        .num_nodes = 255,
        .frequency = 800000
    };
    ws2812_init(&ws2812);
    // int t = 0;
    // int dir = (rand() >> 30) & 1 ? 1 : -1;
    // for (int i = 0; i < 1000; ++i) {
    //     ws2812_pattern_snakes(&ws2812, t);
    //     sleep_ms(10);
    //     t += dir;
    // }
    // ws2812_pattern_greys(&ws2812, 0);

    PICO_LOGI("initialize ws2812 OK");

    PICO_LOGI("initialize regulator");

    regulator_t regulator_en = {
        .gpio = REG_EN_PIN
    };
    regulator_init(&regulator_en);
    regulator_off(&regulator_en);

    PICO_LOGI("initialize regulator OK");

    PICO_LOGI("initialize rgb0");

    RGB_t rgb0 = {
        .gpio_r = RGB0_R_PIN,
        .gpio_g = RGB0_G_PIN,
        .gpio_b = RGB0_B_PIN,
        .r = 0,
        .g = 0,
        .b = 0,
        .brightness = 0
    };
    rgb_init(&rgb0);

    PICO_LOGI("initialize rgb0 OK");

    PICO_LOGI("initialize rgb1");

    RGB_t rgb1 = {
        .gpio_r = RGB1_R_PIN,
        .gpio_g = RGB1_G_PIN,
        .gpio_b = RGB1_B_PIN,
        .r = 0,
        .g = 0,
        .b = 0,
        .brightness = 0
    };
    rgb_init(&rgb1);

    PICO_LOGI("initialize rgb1 OK");
    
    PICO_LOGI("initialize command");
    command_t command = {
        .spi = spi_default,
        .gpio_rx = PICO_DEFAULT_SPI_RX_PIN,
        .gpio_tx = PICO_DEFAULT_SPI_TX_PIN,
        .gpio_sclk = PICO_DEFAULT_SPI_SCK_PIN,
        .gpio_csn = PICO_DEFAULT_SPI_CSN_PIN
    };
    command_init(&command);
    PICO_LOGI("initialize command OK");

    // main event loop
    while(1) {
        PICO_LOGI("read command");
        
        // block until there's a command
        command_read(&command);
        PICO_LOGI("read command OK");
        
        PICO_LOGI("execute command");

        // handle the command
        switch(command.type) {
            case COMMAND_SYNC: {
                command.response = COMMAND_RESPONSE_SYNC;
            } break;
            case COMMAND_STATUS_LED_SET_STATE: {
                status_led_t* status_led;
                command.response = COMMAND_RESPONSE_OK;
                switch(command.args.status_led_set_state.index) {
                    case COMMAND_STATUS_LED_INDEX_GREEN:
                        status_led = &status_led_green;
                        break;
                    case COMMAND_STATUS_LED_INDEX_RED:
                        status_led = &status_led_red;
                        break;
                    default: {
                        status_led = NULL;
                        command.response = COMMAND_RESPONSE_ERROR_BAD_INDEX;
                    } break;
                }
                if(status_led == NULL)
                    continue;

                status_led_set_state(status_led, command.args.status_led_set_state.state);
            } break;

            case COMMAND_STATUS_LED_BLINK: {
                status_led_t* status_led;
                command.response = COMMAND_RESPONSE_OK;
                switch(command.args.status_led_blink.index) {
                    case COMMAND_STATUS_LED_INDEX_GREEN:
                        status_led = &status_led_green;
                        break;
                    case COMMAND_STATUS_LED_INDEX_RED:
                        status_led = &status_led_red;
                        break;
                    default: {
                        status_led = NULL;
                        command.response = COMMAND_RESPONSE_ERROR_BAD_INDEX;
                    } break;
                }

                if(status_led == NULL)
                    continue;

                status_led_set_state(status_led, command.args.status_led_blink.blink_ms);
            } break;

            case COMMAND_REGULATOR_SET_STATE: {
                regulator_t* regulator;
                command.response = COMMAND_RESPONSE_OK;
                switch(command.args.regulator_set_state.index) {
                    case COMMAND_REGULATOR_INDEX_EN:
                        regulator = &regulator_en;
                        break;
                    default: {
                        regulator = NULL;
                        command.response = COMMAND_RESPONSE_ERROR_BAD_INDEX;
                    } break;
                }
                if(regulator == NULL)
                    continue;
                regulator_set_state(regulator, command.args.regulator_set_state.state);
            } break;

            case COMMAND_RGB_SET_COLOR: {
                RGB_t* rgb;
                command.response = COMMAND_RESPONSE_OK;
                switch(command.args.rgb_set_color.index) {
                    case COMMAND_RGB_INDEX_0:
                        rgb = &rgb0;
                        break;
                    case COMMAND_RGB_INDEX_1:
                        rgb = &rgb1;
                        break;
                    default: {
                        rgb = NULL;
                        command.response = COMMAND_RESPONSE_ERROR_BAD_INDEX;
                    }
                }
                if(rgb == NULL)
                    continue;
                
                rgb_set_color(rgb, command.args.rgb_set_color.r, command.args.rgb_set_color.g, command.args.rgb_set_color.b);
            } break;

            case COMMAND_RGB_SET_BRIGHTNESS: {
                RGB_t* rgb;
                command.response = COMMAND_RESPONSE_OK;
                switch(command.args.rgb_set_brightness.index) {
                    case COMMAND_RGB_INDEX_0:
                        rgb = &rgb0;
                        break;
                    case COMMAND_RGB_INDEX_1:
                        rgb = &rgb1;
                        break;
                    default: {
                        rgb = NULL;
                        command.response = COMMAND_RESPONSE_ERROR_BAD_INDEX;
                    }
                }
                if(rgb == NULL)
                    continue;
                
                rgb_set_brightness(rgb, command.args.rgb_set_brightness.brightness);
            } break;

            case COMMAND_RGB_ENABLE: {
                RGB_t* rgb;
                command.response = COMMAND_RESPONSE_OK;
                switch(command.args.rgb_enable.index) {
                    case COMMAND_RGB_INDEX_0:
                        rgb = &rgb0;
                        break;
                    case COMMAND_RGB_INDEX_1:
                        rgb = &rgb1;
                        break;
                    default: {
                        rgb = NULL;
                        command.response = COMMAND_RESPONSE_ERROR_BAD_INDEX;
                    }
                }
                if(rgb == NULL)
                    continue;
                
                rgb_enable(rgb);
            } break;

            case COMMAND_RGB_DISABLE: {
                RGB_t* rgb;
                command.response = COMMAND_RESPONSE_OK;
                switch(command.args.rgb_disable.index) {
                    case COMMAND_RGB_INDEX_0:
                        rgb = &rgb0;
                        break;
                    case COMMAND_RGB_INDEX_1:
                        rgb = &rgb1;
                        break;
                    default: {
                        rgb = NULL;
                        command.response = COMMAND_RESPONSE_ERROR_BAD_INDEX;
                    }
                }
                if(rgb == NULL)
                    continue;
                
                rgb_disable(rgb);
            } break;

            default: {
                command.response = COMMAND_RESPONSE_ERROR_UNKNOWN_COMMAND;
            } break;
        }
        PICO_LOGI("execute command OK");

        PICO_LOGI("reply command %d", command.response);
        command_reply(&command);
        PICO_LOGI("reply command OK");
    }
}