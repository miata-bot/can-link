#ifndef CONEPROJ_CONSOLE_RGB_H
#define CONEPROJ_CONSOLE_RGB_H

#include <esp_system.h>
#include <esp_console.h>
#include <argtable3/argtable3.h>

#include "pico.h"

#define CONSOLE_RGB_INDEX1 1
#define CONSOLE_RGB_INDEX2 2
// COMMAND_RGB_INDEX_0
// COMMAND_RGB_INDEX_1

esp_err_t console_rgb_install();

#endif