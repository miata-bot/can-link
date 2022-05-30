#ifndef CONEPROJ_CONSOLE_MOTOR_H
#define CONEPROJ_CONSOLE_MOTOR_H

#include <esp_system.h>
#include <esp_console.h>
#include <argtable3/argtable3.h>

#include "motor.h"

#define CONSOLE_MOTOR_INDEX1 1
#define CONSOLE_MOTOR_INDEX2 2

esp_err_t console_motor_install();

#endif