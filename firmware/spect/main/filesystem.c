#include <esp_err.h>
#include <esp_log.h>
#include <esp_spi_flash.h>
#include "esp_vfs_fat.h"

static const char *TAG = "SPECT-FS";

const char *base_path = "/flash";
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

esp_err_t flash_init()
{
  ESP_LOGI(TAG, "Mounting FAT filesystem");
  // To mount device we need name of device partition, define base_path
  // and allow format partition in case if it is new one and was not formatted before
  const esp_vfs_fat_mount_config_t mount_config = {
    .max_files = 4,
    .format_if_mount_failed = true,
    .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
  };
  esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(base_path, "storage", &mount_config, &s_wl_handle);
  return err;
}