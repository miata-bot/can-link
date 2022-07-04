#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#define LED_TYPE LED_STRIP_WS2812
#define LED_GPIO 8

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH0_GPIO       (9)
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_LS_CH1_GPIO       (34)
#define LEDC_LS_CH1_CHANNEL    LEDC_CHANNEL_1
#define LEDC_LS_CH2_GPIO       (4)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO       (5)
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM       (2)
#define LEDC_TEST_DUTY         (8191)
#define LEDC_TEST_FADE_TIME    (1000)

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "program.h"
#include "timerutil.h"
#include "argebee.h"

/*
 * This callback function will be called when fade operation has ended
 * Use callback only if you are aware it is being called inside an ISR
 * Otherwise, you can use a semaphore to unblock tasks
 */
static bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
    portBASE_TYPE taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT) {
        SemaphoreHandle_t counting_sem = (SemaphoreHandle_t) user_arg;
        xSemaphoreGiveFromISR(counting_sem, &taskAwoken);
    }

    return (taskAwoken == pdTRUE);
}


void app_main()
{
    ESP_LOGI("ASD", "HELLOL");
    argebee_cfg_t cfg = {};
    argebee_t* engine;
    argebee_init(&cfg, &engine);

    // argebee_debug("loading program %d\n", program_size);
    argebee_load_program(engine, ______interface_table_generator_bytecode, ______interface_table_generator_bytecode_len);
    argebee_debug("starting program %p %d\n", engine, engine->halted);
    int64_t t1 = esp_timer_get_time();
    int64_t t2 = 0;
    // struct timespec t1;
    // struct timespec t2;
    // if(clock_gettime(CLOCK_MONOTONIC, &t1)) {
    //     argebee_error("failed to get time1");
    // }
    while(engine->halted == false) {
        vTaskDelay(0);
        argebee_step(engine);

        // if(clock_gettime(CLOCK_MONOTONIC, &t2)) {
        //     argebee_debug("failed to get time2");
        // }
        t2 = esp_timer_get_time();

        // int elapsedTime = (MILLION * (t2.tv_sec - t1.tv_sec) + t2.tv_nsec - t1.tv_nsec) / 1000;
        int elapsedTime = t2 - t1;
        // struct timespec diff;
        // sub_timespec(t1, t2, &diff);
        if(elapsedTime > 1000) {
            t1 = esp_timer_get_time();
            engine->TM-=1;
            // argebee_debug("dec TM %d\n", engine->TM);
            if(engine->TM == 0) {
                argebee_debug("TM == 0!\r");
                argebee_debug("%10u : %ld.%.9ld - %ld.%.9ld (%ld.%.9ld)  - \r\n",
                              elapsedTime,
                              t2.tv_sec, t2.tv_nsec,
                              t1.tv_sec, t1.tv_nsec,
                              diff.tv_sec, diff.tv_nsec);
            }
        }
    }
    argebee_debug("halt");
    while(1) {
        vTaskDelay(1000);
    }
    // xTaskCreate(test, "test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);

    int ch;

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        {
            .channel    = LEDC_LS_CH0_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH0_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_LS_TIMER,
            .flags.output_invert = 1
        },
        {
            .channel    = LEDC_LS_CH1_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH1_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_LS_TIMER,
            .flags.output_invert = 0
        },
        // {
        //     .channel    = LEDC_LS_CH2_CHANNEL,
        //     .duty       = 0,
        //     .gpio_num   = LEDC_LS_CH2_GPIO,
        //     .speed_mode = LEDC_LS_MODE,
        //     .hpoint     = 0,
        //     .timer_sel  = LEDC_LS_TIMER,
        //     .flags.output_invert = 1
        // },
        // {
        //     .channel    = LEDC_LS_CH3_CHANNEL,
        //     .duty       = 0,
        //     .gpio_num   = LEDC_LS_CH3_GPIO,
        //     .speed_mode = LEDC_LS_MODE,
        //     .hpoint     = 0,
        //     .timer_sel  = LEDC_LS_TIMER,
        //     .flags.output_invert = 1
        // },
    };

    // Set LED Controller with previously prepared configuration
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);
    ledc_cbs_t callbacks = {
        .fade_cb = cb_ledc_fade_end_event
    };
    SemaphoreHandle_t counting_sem = xSemaphoreCreateCounting(LEDC_TEST_CH_NUM, 0);

    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_cb_register(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, &callbacks, (void *) counting_sem);
    }

    while (1) {
        printf("1. LEDC fade up to duty = %d\n", LEDC_TEST_DUTY);
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                                    ledc_channel[ch].channel, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                            ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }

        for (int i = 0; i < LEDC_TEST_CH_NUM; i++) {
            xSemaphoreTake(counting_sem, portMAX_DELAY);
        }

        printf("2. LEDC fade down to duty = 0");
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                                    ledc_channel[ch].channel, 0, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                            ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }

        for (int i = 0; i < LEDC_TEST_CH_NUM; i++) {
            xSemaphoreTake(counting_sem, portMAX_DELAY);
        }

        // printf("3. LEDC set duty = %d without fade\n", LEDC_TEST_DUTY);
        // for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        //     ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, LEDC_TEST_DUTY);
        //     ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        // }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);

        // printf("4. LEDC set duty = 0 without fade");
        // for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        //     ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, 0);
        //     ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        // }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

