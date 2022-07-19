 #include <led_strip.h>
#include <driver/ledc.h>

#include "gatt_srv.h"
#include "esp_ota_ops.h"

#include "spect-config.h"

static const char* TAG = "BLE";

spect_config_context_t* config_ctx;

static uint32_t addressable_led_channel_1_color = 0;
static uint32_t addressable_led_channel_2_color = 0;

static uint32_t rgb_led_channel_1_color = 0;
static uint32_t rgb_led_channel_2_color = 0;

static spect_mode_t tmp_mode;

uint8_t gatt_svr_chr_ota_control_val;
uint8_t gatt_svr_chr_ota_data_val[512];

uint16_t ota_control_val_handle;
uint16_t ota_data_val_handle;

const esp_partition_t *update_partition;
esp_ota_handle_t update_handle;
bool updating = false;
uint16_t num_pkgs_received = 0;
uint16_t packet_size = 0;

static int
state_mode_handle_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
        return BLE_ATT_ERR_UNLIKELY;
    
    uint8_t* mode = (uint8_t*)dst;
    if(*mode >= SPECT_MODE_MAX) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

    spect_set_mode(config_ctx, *mode);
    return 0;
}

static int state_mode_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid_t *uuid = ctxt->chr->uuid;
    int rc;

    // addressable channel 1
    if (ble_uuid_cmp(uuid, &state_mode_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &config_ctx->config->state->mode,
                                sizeof config_ctx->config->state->mode);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = state_mode_handle_write(ctxt->om,
                                         sizeof config_ctx->config->state->mode,
                                         sizeof config_ctx->config->state->mode,
                                         &tmp_mode, NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        // service: OTA Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_ota_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    // characteristic: OTA control
                    .uuid = &gatt_svr_chr_ota_control_uuid.u,
                    .access_cb = gatt_svr_chr_ota_control_cb,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                             BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &ota_control_val_handle,
                },
                {
                    // characteristic: OTA data
                    .uuid = &gatt_svr_chr_ota_data_uuid.u,
                    .access_cb = gatt_svr_chr_ota_data_cb,
                    .flags = BLE_GATT_CHR_F_WRITE,
                    .val_handle = &ota_data_val_handle,
                },
                {
                    0,
                }},
    },
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &state_service.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {   {
                /** Characteristic: Addressable LED Channel 1 */
                .uuid = &state_mode_characteristic.u,
                .access_cb = state_mode_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            }, {
                /** Characteristic: Addressable RGB Channel 1 */
                .uuid = &addressable_led_channel_1_characteristic.u,
                .access_cb = led_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            }, 
            {
                0, /* No more characteristics in this service. */
            }
        },
    }, {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &config_service.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {   {
                /** Characteristic: strip length */
                .uuid = &config_strip_length_characteristic.u,
                .access_cb = config_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            }, 
            {
                0, /* No more characteristics in this service. */
            }
        },
    }, {
        0, /* No more services. */
    },
};

static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                              uint16_t max_len, void *dst, uint16_t *len) {
  uint16_t om_len;
  int rc;

  om_len = OS_MBUF_PKTLEN(om);
  if (om_len < min_len || om_len > max_len) {
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
  }

  rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
  if (rc != 0) {
    return BLE_ATT_ERR_UNLIKELY;
  }

  return 0;
}

static void update_ota_control(uint16_t conn_handle) {
  struct os_mbuf *om;
  esp_err_t err;
  ESP_LOGI(TAG, "update_ota_control");

  // check which value has been received
  switch (gatt_svr_chr_ota_control_val) {
    case SVR_CHR_OTA_CONTROL_REQUEST:
      // OTA request
      ESP_LOGI(TAG, "OTA has been requested via BLE.");
      // get the next free OTA partition
      update_partition = esp_ota_get_next_update_partition(NULL);
      // start the ota update
      err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES,
                          &update_handle);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)",
                 esp_err_to_name(err));
        esp_ota_abort(update_handle);
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_NAK;
      } else {
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_ACK;
        updating = true;

        // retrieve the packet size from OTA data
        packet_size =
            (gatt_svr_chr_ota_data_val[1] << 8) + gatt_svr_chr_ota_data_val[0];
        ESP_LOGI(TAG, "Packet size is: %d", packet_size);

        num_pkgs_received = 0;
      }

      // notify the client via BLE that the OTA has been acknowledged (or not)
      om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                 sizeof(gatt_svr_chr_ota_control_val));
      ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
      ESP_LOGI(TAG, "OTA request acknowledgement has been sent.");

      break;

    case SVR_CHR_OTA_CONTROL_DONE:

      updating = false;

      // end the OTA and start validation
      err = esp_ota_end(update_handle);
      if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
          ESP_LOGE(TAG,
                   "Image validation failed, image is corrupted!");
        } else {
          ESP_LOGE(TAG, "esp_ota_end failed (%s)!",
                   esp_err_to_name(err));
        }
      } else {
        // select the new partition for the next boot
        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK) {
          ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!",
                   esp_err_to_name(err));
        }
      }

      // set the control value
      if (err != ESP_OK) {
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_NAK;
      } else {
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_ACK;
      }

      // notify the client via BLE that DONE has been acknowledged
      om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                 sizeof(gatt_svr_chr_ota_control_val));
      ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
      ESP_LOGI(TAG, "OTA DONE acknowledgement has been sent.");

      // restart the ESP to finish the OTA
      if (err == ESP_OK) {
        ESP_LOGI(TAG, "Preparing to restart!");
        vTaskDelay(pdMS_TO_TICKS(REBOOT_DEEP_SLEEP_TIMEOUT));
        esp_restart();
      }

      break;

    default:
      break;
  }
}

static int gatt_svr_chr_ota_control_cb(uint16_t conn_handle,
                                       uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt *ctxt,
                                       void *arg) {
  ESP_LOGI(TAG, "gatt_svr_chr_ota_control_cb");
  int rc;
  uint8_t length = sizeof(gatt_svr_chr_ota_control_val);

  switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
      // a client is reading the current value of ota control
      rc = os_mbuf_append(ctxt->om, &gatt_svr_chr_ota_control_val, length);
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
      break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
      // a client is writing a value to ota control
      rc = gatt_svr_chr_write(ctxt->om, 1, length,
                              &gatt_svr_chr_ota_control_val, NULL);
      // update the OTA state with the new value
      update_ota_control(conn_handle);
      return rc;
      break;

    default:
      break;
  }

  // this shouldn't happen
  assert(0);
  return BLE_ATT_ERR_UNLIKELY;
}

static int gatt_svr_chr_ota_data_cb(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt,
                                    void *arg) {
  int rc;
  esp_err_t err;
  ESP_LOGI(TAG, "gatt_svr_chr_ota_data_cb");

  // store the received data into gatt_svr_chr_ota_data_val
  rc = gatt_svr_chr_write(ctxt->om, 1, sizeof(gatt_svr_chr_ota_data_val),
                          gatt_svr_chr_ota_data_val, NULL);

  // write the received packet to the partition
  if (updating) {
    err = esp_ota_write(update_handle, (const void *)gatt_svr_chr_ota_data_val,
                        packet_size);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "esp_ota_write failed (%s)!",
               esp_err_to_name(err));
    }

    num_pkgs_received++;
    ESP_LOGI(TAG, "Received packet %d", num_pkgs_received);
  }

  ESP_LOGI(TAG, "gatt_svr_chr_ota_data_cb ok");
  return rc;
}

static int
config_handle_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                             void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
        return BLE_ATT_ERR_UNLIKELY;

    ESP_LOGI("BLE", "config set");
    esp_err_t err = spect_set_config(config_ctx);
    ESP_ERROR_CHECK(err);
    return 0;
}

static int
addressable_led_handle_write(led_strip_t* strip, struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                             void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
        return BLE_ATT_ERR_UNLIKELY;
    
    uint8_t* color = (uint8_t*)dst;
    config_ctx->config->state->data.solid.channel0 = color[0] + (color[1] << 8) + (color[2] << 16) + (color[3] << 24);
    spect_set_state(config_ctx);
    rgb_t color_;
    color_.red = color[1];
    color_.green = color[0];
    color_.blue = color[2];
    ESP_LOGE("LED", "fill %02X %02X %02X", color_.red, color_.green, color_.blue);
    led_strip_fill(config_ctx->rgb0->strip, 0, config_ctx->rgb0->strip->length, color_);
    led_strip_wait(config_ctx->rgb0->strip, 1000);
    led_strip_flush(config_ctx->rgb0->strip);
    return 0;
}

static int
rgb_led_handle_write(ledc_channel_config_t* ledc_r, ledc_channel_config_t* ledc_g, ledc_channel_config_t* ledc_b,  
                     struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
        return BLE_ATT_ERR_UNLIKELY;

    uint8_t* color = (uint8_t*)dst;
    ESP_LOGE("RGB", "fill %02X %02X %02X", color[0], color[1], color[2]);
    return 0;
}

static int
led_access(uint16_t conn_handle, uint16_t attr_handle,
           struct ble_gatt_access_ctxt *ctxt,
           void *arg)
{
    const ble_uuid_t *uuid = ctxt->chr->uuid;
    int rc;

    // addressable channel 1
    if (ble_uuid_cmp(uuid, &addressable_led_channel_1_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &addressable_led_channel_1_color,
                                sizeof addressable_led_channel_1_color);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = addressable_led_handle_write(NULL, ctxt->om,
                                              sizeof addressable_led_channel_1_color,
                                              sizeof addressable_led_channel_1_color,
                                              &addressable_led_channel_1_color, NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    // addressable channel 2
    if (ble_uuid_cmp(uuid, &addressable_led_channel_2_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &addressable_led_channel_2_color,
                                sizeof addressable_led_channel_2_color);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = addressable_led_handle_write(NULL, ctxt->om,
                                              sizeof addressable_led_channel_2_color,
                                              sizeof addressable_led_channel_2_color,
                                              &addressable_led_channel_2_color, NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    // rgb channel 1
    if (ble_uuid_cmp(uuid, &rgb_led_channel_1_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &rgb_led_channel_1_color,
                                sizeof rgb_led_channel_1_color);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            if(config_ctx->config->state->mode != SPECT_MODE_EFFECT_SOLID) return BLE_ATT_ERR_INSUFFICIENT_RES;
            rc = rgb_led_handle_write(NULL, NULL, NULL, 
                                              ctxt->om,
                                              sizeof rgb_led_channel_1_color,
                                              sizeof rgb_led_channel_1_color,
                                              &rgb_led_channel_1_color, NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    // rgb channel 2
    if (ble_uuid_cmp(uuid, &rgb_led_channel_2_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &rgb_led_channel_2_color,
                                sizeof rgb_led_channel_2_color);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = rgb_led_handle_write(NULL, NULL, NULL, 
                                            ctxt->om,
                                              sizeof rgb_led_channel_2_color,
                                              sizeof rgb_led_channel_2_color,
                                              &rgb_led_channel_2_color, NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int config_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg)
{
    const ble_uuid_t *uuid = ctxt->chr->uuid;
    int rc;

    // addressable channel 1
    if (ble_uuid_cmp(uuid, &config_strip_length_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &config_ctx->config->strip_channel_1_length,
                                sizeof config_ctx->config->strip_channel_1_length);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = config_handle_write(ctxt->om,
                                     sizeof(uint8_t),
                                     sizeof config_ctx->config->strip_channel_1_length,
                                     &config_ctx->config->strip_channel_1_length, NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }
    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void
gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int
gatt_svr_init(spect_config_context_t* config_ctx_)
{
    config_ctx = config_ctx_;

    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_ans_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }
    ESP_LOGE("BLE", "ble init ok");

    return 0;
}
