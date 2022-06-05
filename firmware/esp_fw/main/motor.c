#include "motor.h"
static const char *TAG = "MOTOR";
enum mode {
    DECAY_SLOW,
    DECAY_FAST
};
enum mode decay_mode = DECAY_SLOW;

void motor_set_duty(motor_t* motor, float duty_cycle)
{
    ESP_LOGI(TAG, "motor dc=%f ina=%d inb=%d ena=%d enb=%d", duty_cycle, motor->cfg.gpio_inA, motor->cfg.gpio_inB, motor->cfg.gpio_enA, motor->cfg.gpio_enB);
    /* motor moves in forward direction, with duty cycle = duty % */
    if (duty_cycle > 0) {
        if(decay_mode == DECAY_SLOW) {
            gpio_set_level(motor->cfg.gpio_enA, 1);
            gpio_set_level(motor->cfg.gpio_enB, 1);
        } else {
            gpio_set_level(motor->cfg.gpio_inA, 1);
            gpio_set_level(motor->cfg.gpio_inB, 0);
        }
        mcpwm_set_signal_low(BDC_MCPWM_UNIT, BDC_MCPWM_TIMER, MCPWM_OPR_A);
        mcpwm_set_duty(BDC_MCPWM_UNIT, BDC_MCPWM_TIMER, MCPWM_OPR_B, duty_cycle);
        mcpwm_set_duty_type(BDC_MCPWM_UNIT, BDC_MCPWM_TIMER, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
    }
    /* motor moves in backward direction, with duty cycle = -duty % */
    else {
        if(decay_mode == DECAY_SLOW) {
            gpio_set_level(motor->cfg.gpio_enA, 1);
            gpio_set_level(motor->cfg.gpio_enB, 1);
        } else {
            gpio_set_level(motor->cfg.gpio_inA, 0);
            gpio_set_level(motor->cfg.gpio_inB, 1);
        }

        mcpwm_set_signal_low(BDC_MCPWM_UNIT, BDC_MCPWM_TIMER, MCPWM_OPR_B);
        mcpwm_set_duty(BDC_MCPWM_UNIT, BDC_MCPWM_TIMER, MCPWM_OPR_A, -duty_cycle);
        mcpwm_set_duty_type(BDC_MCPWM_UNIT, BDC_MCPWM_TIMER, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    }
}

esp_err_t motor_initialize(motor_config_t* cfg, motor_t** out_ctx)
{
    ESP_LOGI(TAG, "configure mcpwm gpio inA=%d inB=%d enA=%d enB=%d", cfg->gpio_inA, cfg->gpio_inB, cfg->gpio_enA, cfg->gpio_enB);
    esp_err_t err = ESP_OK;
    // err = mcpwm_gpio_init(cfg->unit, MCPWM0A, cfg->gpio_inA);
    // if(err != ESP_OK)
    //   return err;

    // err = mcpwm_gpio_init(cfg->unit, MCPWM0B, cfg->gpio_inB);
    // if(err != ESP_OK)
    //   return err;

    if(decay_mode == DECAY_SLOW) {
        ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0A, cfg->gpio_inA));
        ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0B, cfg->gpio_inB));

        gpio_set_direction(cfg->gpio_enA, GPIO_MODE_OUTPUT);
        gpio_set_level(cfg->gpio_enA, 1);
        gpio_set_direction(cfg->gpio_enB, GPIO_MODE_OUTPUT);
        gpio_set_level(cfg->gpio_enB, 1);
    } else {
        ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0A, cfg->gpio_enA));
        ESP_ERROR_CHECK(mcpwm_gpio_init(BDC_MCPWM_UNIT, MCPWM0B, cfg->gpio_enB));

        gpio_set_direction(cfg->gpio_inA, GPIO_MODE_OUTPUT);
        gpio_set_level(cfg->gpio_inA, 0);
        gpio_set_direction(cfg->gpio_inB, GPIO_MODE_OUTPUT);
        gpio_set_level(cfg->gpio_inB, 0);
    }

    mcpwm_config_t pwm_config = {
        .frequency = BDC_MCPWM_FREQ_HZ,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    ESP_ERROR_CHECK(mcpwm_init(BDC_MCPWM_UNIT, BDC_MCPWM_TIMER, &pwm_config));

    // ESP_LOGI(TAG, "init mcpwm driver");
    // mcpwm_config_t pwm_config = {
    //     .frequency = BDC_MCPWM_FREQ_HZ,
    //     .cmpr_a = 0,
    //     .cmpr_b = 0,
    //     .counter_mode = MCPWM_UP_COUNTER,
    //     .duty_mode = MCPWM_DUTY_MODE_0,
    // };
    // err = mcpwm_init(cfg->unit, cfg->timer, &pwm_config);
    // if(err != ESP_OK)
    //   return err;

    motor_t* ctx = (motor_t*)malloc(sizeof(motor_t));
    if (!ctx) return ESP_ERR_NO_MEM;

    *ctx = (motor_t) {
        .cfg = *cfg,
    };

    *out_ctx = ctx;
    return ESP_OK;
}

void motor_deinit()
{
}

