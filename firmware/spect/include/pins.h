#ifndef CONEPROJ_PINS_H
#define CONEPROJ_PINS_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/spi_types.h"

#define GPIO_NUM_SX1231_CSN (0)
#define GPIO_NUM_SX1231_IRQ (1)
#define GPIO_NUM_SX1231_RESET (2)
#define GPIO_NUM_DI3 (3)
#define GPIO_NUM_DI1 (4)
#define GPIO_NUM_DI4 (5)
#define GPIO_NUM_DI2 (6)

/**
 * @brief Select Array for Digital Input
 */
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_NUM_DI3) | \
                             (1ULL<<GPIO_NUM_DI1) | \
                             (1ULL<<GPIO_NUM_DI4) | \
                             (1ULL<<GPIO_NUM_DI2))

#define ESP_INTR_FLAG_DEFAULT 0

#define GPIO_NUM_VSENSE (7)
#define GPIO_NUM_STRIP1 (8)
#define GPIO_NUM_STRIP0 (9)

#define GPIO_NUM_TWAI_EN (10)
#define GPIO_NUM_TWAI_TX (11)
#define GPIO_NUM_TWAI_RX (12)

#define GPIO_NUM_RGB1_B (13)
#define GPIO_NUM_RGB1_G (14)
#define GPIO_NUM_RGB1_R (15)

#define GPIO_NUM_RGB0_B (16)
#define GPIO_NUM_RGB0_G (17)
#define GPIO_NUM_RGB0_R (18)

#define GPIO_NUM_RGB0_EN (26)
#define GPIO_NUM_RGB1_EN (27)
/**
 * @brief Select array for output GPIOS
 * NOTE: GPIO_NUM_RGB1_EN is not included here. It causes a boot loop
 *       WRT RTC watchdog or something like that. Did not investigate further.
 */
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_NUM_RGB0_EN) | \
                              (1ULL<<GPIO_NUM_RGB0_R)  | \
                              (1ULL<<GPIO_NUM_RGB0_G)  | \
                              (1ULL<<GPIO_NUM_RGB0_B)  | \
                              (1ULL<<GPIO_NUM_RGB1_R)  | \
                              (1ULL<<GPIO_NUM_RGB1_G)  | \
                              (1ULL<<GPIO_NUM_RGB1_B)  | \
                              (1ULL<<GPIO_NUM_TWAI_EN))

// SPI
#define GPIO_NUM_SPI2_SCLK (35)
#define GPIO_NUM_SPI2_MOSI (36)
#define GPIO_NUM_SPI2_MISO (37)
#define SPI_HOST           SPI2_HOST
#define SPI_DMA_CHANNEL    SPI_DMA_CH_AUTO

// LEDC
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE

#define LEDC_LS_CH0_GPIO       (GPIO_NUM_RGB0_R) // red
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0

#define LEDC_LS_CH1_GPIO       (GPIO_NUM_RGB0_G) // green
#define LEDC_LS_CH1_CHANNEL    LEDC_CHANNEL_1

#define LEDC_LS_CH2_GPIO       (GPIO_NUM_RGB0_B) // blue
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2

#define LEDC_LS_CH3_GPIO       (GPIO_NUM_RGB1_R) // red
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3

#define LEDC_LS_CH4_GPIO       (GPIO_NUM_RGB1_G) // green
#define LEDC_LS_CH4_CHANNEL    LEDC_CHANNEL_4

#define LEDC_LS_CH5_GPIO       (GPIO_NUM_RGB1_B) // blue
#define LEDC_LS_CH5_CHANNEL    LEDC_CHANNEL_5

/**
 * @brief total Number of LEDC channels: 6, RGB1, RGB2
 */
#define LEDC_TEST_CH_NUM       (6)

#endif