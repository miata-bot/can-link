#include <esp_err.h>
#include <esp_log.h>
#include <esp_spi_flash.h>
#include "esp_vfs_fat.h"

#include <string.h>
// #include "ff.h"
#include <dirent.h>
static const char *TAG = "SPECT-FS";

const char *base_path = "/flash";
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

  /* Start node to be scanned (***also used as work area***) */
void scan_files(char* path)
{
  DIR *dp;
  struct dirent *ep;     
  dp = opendir (path);
  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL)
      ESP_LOGI ("FS", "%s", ep->d_name);
          
    (void) closedir (dp);
    return;
  }
  else
  {
    ESP_LOGE ("FS", "Couldn't open the directory");
  }
  // FRESULT res;
  // DIR dir;
  // UINT i;
  // static FILINFO fno;

  // res = opendir(path);                       /* Open the directory */
  // if (res == FR_OK) {
  //     for (;;) {
  //         res = readdir(&dir, &fno);                   /* Read a directory item */
  //         if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
  //         if (fno.fattrib & AM_DIR) {                    /* It is a directory */
  //             i = strlen(path);
  //             sprintf(&path[i], "/%s", fno.fname);
  //             res = scan_files(path);                    /* Enter the directory */
  //             if (res != FR_OK) break;
  //             path[i] = 0;
  //         } else {                                       /* It is a file. */
  //             ESP_LOGI("FS", "%s/%s\n", path, fno.fname);
  //         }
  //     }
  //     closedir(&dir);
  //     ESP_LOGI("FS", "scan ok");
  // } else {
  //   ESP_LOGE("FS", "could not list dir %d", res);
  // }



  // return res;
}

esp_err_t flash_init()
{
  ESP_LOGI(TAG, "Mounting FAT filesystem");
  // To mount device we need name of device partition, define base_path
  // and allow format partition in case if it is new one and was not formatted before
  const esp_vfs_fat_mount_config_t mount_config = {
    .max_files = 4,
    .format_if_mount_failed = false,
    .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
  };
  esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(base_path, "storage", &mount_config, &s_wl_handle);
  if(err != ESP_OK) {
    ESP_LOGE("FS", "error mounting flash");
  }
  scan_files("/flash");
  return err;
}