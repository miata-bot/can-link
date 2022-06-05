#include "console_motor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

static const char *TAG = "CONSOLE_MOTOR";

extern motor_t* motor1;
extern motor_t* motor2;

static struct {
    struct arg_int *index;
    struct arg_dbl *duty;
    struct arg_end *end;
} motor_args;

static int console_motor(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &motor_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, motor_args.end, argv[0]);
        return 1;
    }
    if(motor_args.index->ival[0] == CONSOLE_MOTOR_INDEX1) {
        motor_set_duty(motor1, motor_args.duty->dval[0]);
        return 0;
    }

    if (motor_args.index->ival[0] == CONSOLE_MOTOR_INDEX2) {
        // motor_set_duty(motor2, motor_args.duty->dval[0]);
        for(int i = 0; i < 25; i++) {
            motor_set_duty(motor1, i);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        for(int i = 25; i > 0; i--) {
            motor_set_duty(motor1, i);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }


        return 0;
    }
    ESP_LOGE(TAG, "unknown motor index %d", motor_args.index->ival[0]);
    return 1;
}

esp_err_t console_motor_install()
{
    motor_args.index = arg_intn("i", "index", "1|2", 0,1, "motor index 1 or 2");
    motor_args.index->ival[0] = CONSOLE_MOTOR_INDEX1;

    motor_args.duty = arg_dbln("d", "duty", "<f32>", 1, 1, "motor duty percent 0-100");
    motor_args.end = arg_end(1);

    const esp_console_cmd_t motor_cmd = {
        .command = "motor",
        .help = "set the motor duty",
        .hint = NULL,
        .func = &console_motor,
        .argtable = &motor_args
    };
    return esp_console_cmd_register(&motor_cmd);
}