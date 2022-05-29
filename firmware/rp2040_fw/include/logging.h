#ifndef CONEPROJ_LOGGING_H
#define CONEPROJ_LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DEBUG

#ifdef DEBUG
#define PICO_LOGI(...) do { printf(__VA_ARGS__); printf("\r\n"); } while(0)
#else
#define PICO_LOGI(...)
#endif

#endif