#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "tusb.h"
#include "msc.h"
#include "util.h"
#include "sdkconfig.h"

static const char *TAG = "MSC";
static bool ejected = false;
const esp_partition_t *partition;
wl_handle_t wl_handle;

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    (void) lun;

    const char vid[8] = "CONERGB";
    const char pid[16] = "Flash Storage";
    const char rev[4] = "0.1";

    ESP_LOGD(TAG, "tud_msc_inquiry_cb() invoked");

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void) lun;

    ESP_LOGD(TAG, "tud_msc_test_unit_ready_cb() invoked");

    if (ejected) {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
        return false;
    }

    return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void) lun;
    *block_count = wl_size(wl_handle) / wl_sector_size(wl_handle);
    *block_size  = wl_sector_size(wl_handle);
    ESP_LOGD(TAG, "tud_msc_capacity_cb()  block_count=%d block_size=%d", *block_count, *block_size);
}

// this isn't called when i think it should be.
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void) lun;
    (void) power_condition;

    ESP_LOGD(TAG, "tud_msc_start_stop_cb() invoked, power_condition=%d, start=%d, load_eject=%d", power_condition,
             start, load_eject);
    if (load_eject) {
        if (start) {
            // load disk storage
        } else {
            if(ejected)
                return true;

            // unload disk storage
            ejected = true;
            esp_err_t err = wl_unmount(wl_handle);
            ESP_ERROR_CHECK(err);
            ESP_LOGD(TAG, "unmount ok");
        }
    }

    return true;
}

// Invoked when received SCSI READ10 command
// - Address = lba * BLOCK_SIZE + offset
//   - offset is only needed if CFG_TUD_MSC_EP_BUFSIZE is smaller than BLOCK_SIZE.
//
// - Application fill the buffer (up to bufsize) with address contents and return number of read byte. If
//   - read < bufsize : These bytes are transferred first and callback invoked again for remaining data.
//
//   - read == 0      : Indicate application is not ready yet e.g disk I/O busy.
//                      Callback invoked again with the same parameters later on.
//
//   - read < 0       : Indicate application error e.g invalid address. This request will be STALLed
//                      and return failed status in command status wrapper phase.

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    ESP_LOGD(TAG, "tud_msc_read10_cb() invoked, lun=%d, lba=%d, offset=%d, bufsize=%d", lun, lba, offset, bufsize);
    esp_err_t err = wl_read(wl_handle, lba * wl_sector_size(wl_handle) + offset, buffer, bufsize);
    ESP_ERROR_CHECK(err);
    ESP_LOG_BUFFER_HEXDUMP("tud_msc_read10_cb", buffer, bufsize, ESP_LOG_DEBUG);
    return bufsize;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    // Linux and Windows write files differently. Windows also creates system volume information files on the first
    // mount. In an ideal case, FAT and ROOT content would be analyzed and the flash file detected.
    // However, the only reliable way to detect files for flashing is look at the content.
    // ESP_LOGD(TAG, "tud_msc_write10_cb() invoked, lun=%d, lba=%d, offset=%d, bufsize=%d", lun, lba, offset, bufsize);
    // ESP_LOG_BUFFER_HEXDUMP("tud_msc_write10_cb", buffer, bufsize, ESP_LOG_DEBUG);

    esp_err_t err = wl_erase_range(wl_handle, lba * wl_sector_size(wl_handle) + offset, bufsize);
    ESP_ERROR_CHECK(err);
    err =  wl_write(wl_handle, lba * wl_sector_size(wl_handle) + offset, buffer, bufsize);
    ESP_ERROR_CHECK(err);

    return bufsize;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    int32_t ret;

    ESP_LOGD(TAG, "tud_msc_scsi_cb() invoked. bufsize=%d", bufsize);
    ESP_LOG_BUFFER_HEXDUMP("scsi_cmd", scsi_cmd, 16, ESP_LOG_DEBUG);

    switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        ESP_LOGD(TAG, "tud_msc_scsi_cb() invoked: SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL");
        ret = 0;
        break;

    default:
        ESP_LOGW(TAG, "tud_msc_scsi_cb() invoked: %d", scsi_cmd[0]);

        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

        ret = -1;
        break;
    }

    return ret;
}

// initializes the storage partition
void msc_task(void *pvParameters)
{
    partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "storage");
    if(partition == NULL) {
        ESP_LOGE(TAG, "Could not find storage partition. Maybe it didn't get flashed?");
        eub_abort();
    }
    ESP_LOGI(TAG, "Storage partition found");
    esp_err_t err = wl_mount(partition, &wl_handle);
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "Storage partition mounted");
    vTaskDelete(NULL);
}
