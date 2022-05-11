#include "sdcard.h"

// SD Config
#define PIN_NUM_SD_CS 15
const char SD_MOUNT_POINT[] = "/sdcard";
sdmmc_card_t SDCARD;

static const char *SDCARD_TAG = "CONEPROJ";

void sdcard_init()
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    ESP_LOGI(SDCARD_TAG, "Mounting SD filesystem");

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_SD_CS;
    slot_config.host_id = host.slot;
    sdmmc_card_t *card = &SDCARD;

    // Mount the filesystem
    esp_err_t ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    switch (ret)
    {
    case ESP_OK:
        ESP_LOGI(SDCARD_TAG, "Filesystem mounted");
        break;
    case ESP_FAIL:
        ESP_LOGE(SDCARD_TAG, "Failed to mount filesystem.");
        return;
    default:
        ESP_LOGE(SDCARD_TAG, "Failed to initialize the card (%s). ", esp_err_to_name(ret));
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

void sdcard_deinit()
{
    // unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, &SDCARD);
}