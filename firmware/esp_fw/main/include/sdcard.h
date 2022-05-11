#ifndef CONEPROJ_SDCARD_H
#define CONEPROJ_SDCARD_H

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <driver/twai.h>
#include <soc/gpio_struct.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>

void sdcard_init();
void sdcard_deinit();

#endif