#ifndef CONEPROJ_MOTOR_H
#define CONEPROJ_MOTOR_H

#include <stdint.h>

#include <esp_log.h>
#include <esp_system.h>
#include <driver/mcpwm.h>

#define BDC_MCPWM_UNIT                0
#define BDC_MCPWM_TIMER               0
#define BDC_MCPWM_FREQ_HZ             1500

typedef struct {
    gpio_num_t gpio_inA;
    gpio_num_t gpio_enA;

    gpio_num_t gpio_inB;
    gpio_num_t gpio_enB;
    uint8_t    unit;
    uint8_t    timer;
    uint32_t   frequency;
} motor_config_t;

typedef struct motor {
    motor_config_t cfg;
    float duty;
} motor_t;

esp_err_t motor_initialize(motor_config_t*, motor_t**);
void motor_deinit();

void motor_set_duty(motor_t*, float);

#endif