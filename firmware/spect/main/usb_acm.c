#include "usb_acm.h"
#include "esp_ota_ops.h"

static const char *TAG = "ACM";

typedef enum {
  SVR_CHR_OTA_CONTROL_NOP=0x0,
  SVR_CHR_OTA_CONTROL_REQUEST,
  SVR_CHR_OTA_CONTROL_REQUEST_ACK,
  SVR_CHR_OTA_CONTROL_REQUEST_NAK,
  SVR_CHR_OTA_CONTROL_DONE,
  SVR_CHR_OTA_CONTROL_DONE_ACK,
  SVR_CHR_OTA_CONTROL_DONE_NAK,
  SVR_CHR_OTA_WRITE
} svr_chr_ota_control_val_t;

static uint8_t buf[CFG_TUD_CDC_RX_BUFSIZE + 1];

uint16_t ota_control_val_handle;
uint16_t ota_data_val_handle;

const esp_partition_t *update_partition;
esp_ota_handle_t update_handle;
bool updating = false;
uint16_t num_pkgs_received = 0;
uint16_t packet_size = 0;
svr_chr_ota_control_val_t ctrl;

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CFG_TUD_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Data from channel %d:", itf);
        ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
    } else {
        ESP_LOGE(TAG, "Read error");
        return;
    }

    ctrl = buf[0];
     // check which value has been received
    switch (ctrl) {
    case SVR_CHR_OTA_CONTROL_REQUEST:
      // OTA request
      ESP_LOGI(TAG, "OTA has been requested");
      // get the next free OTA partition
      update_partition = esp_ota_get_next_update_partition(NULL);
      // start the ota update
      ret = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES,
                          &update_handle);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(ret));
        esp_ota_abort(update_handle);
        ctrl = SVR_CHR_OTA_CONTROL_REQUEST_NAK;
      } else {
        ctrl = SVR_CHR_OTA_CONTROL_REQUEST_ACK;
        updating = true;

        // retrieve the packet size from OTA data
        packet_size = (buf[2] << 8) + buf[1];
        assert(packet_size < CFG_TUD_CDC_RX_BUFSIZE+1);
        ESP_LOGI(TAG, "Packet size is: %d", packet_size);

        num_pkgs_received = 0;
      }

      // notify the client via BLE that the OTA has been acknowledged (or not)

      ESP_LOGI(TAG, "OTA request acknowledgement has been sent.");

      break;

    case SVR_CHR_OTA_CONTROL_DONE:

      updating = false;

      // end the OTA and start validation
      ret = esp_ota_end(update_handle);
      if (ret != ESP_OK) {
        if (ret == ESP_ERR_OTA_VALIDATE_FAILED) {
          ESP_LOGE(TAG,
                   "Image validation failed, image is corrupted!");
        } else {
          ESP_LOGE(TAG, "esp_ota_end failed (%s)!",
                   esp_err_to_name(ret));
        }
      } else {
        // select the new partition for the next boot
        ret = esp_ota_set_boot_partition(update_partition);
        if (ret != ESP_OK) {
          ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!",
                   esp_err_to_name(ret));
        }
      }

      // set the control value
      if (ret != ESP_OK) {
        ctrl = SVR_CHR_OTA_CONTROL_DONE_NAK;
      } else {
        ctrl = SVR_CHR_OTA_CONTROL_DONE_ACK;
      }

      // notify the client via BLE that DONE has been acknowledged
    
      ESP_LOGI(TAG, "OTA DONE acknowledgement has been sent.");

      // restart the ESP to finish the OTA
      if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Preparing to restart!");
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();
      }

      break;
    case SVR_CHR_OTA_WRITE:
      if (updating) {
        ret = esp_ota_write(update_handle, (const void *)buf+1, packet_size);
        if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_write failed (%s)!", esp_err_to_name(ret));}
        num_pkgs_received++;
        ESP_LOGI(TAG, "Received packet %d", num_pkgs_received);
      }
      break;

    default:
      break;
  }

    /* write back */
    // tinyusb_cdcacm_write_queue(itf, buf, rx_size);
    // tinyusb_cdcacm_write_flush(itf, 0);
}

void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

void acm_init()
{
    ESP_LOGI(TAG, "initialization");
    // tinyusb_config_t tusb_cfg = {}; // the configuration using default values
    // ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t amc_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&amc_cfg));
    /* the second way to register a callback */
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_0,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &tinyusb_cdc_line_state_changed_callback));

#if (CONFIG_TINYUSB_CDC_COUNT > 1)
    amc_cfg.cdc_port = TINYUSB_CDC_ACM_1;
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&amc_cfg));
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_1,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &tinyusb_cdc_line_state_changed_callback));
#endif

    ESP_LOGI(TAG, "initialization OK");
}