#include <led_strip.h>
#include <driver/ledc.h>

#include "gatt_srv.h"

#include "spect-config.h"

spect_config_context_t* config_ctx;

static uint32_t addressable_led_channel_1_color = 0;
static uint32_t addressable_led_channel_2_color = 0;

static uint32_t rgb_led_channel_1_color = 0;
static uint32_t rgb_led_channel_2_color = 0;

static spect_mode_t tmp_mode;

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
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &state_service.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {   {
                /** Characteristic: Addressable LED Channel 1 */
                .uuid = &state_mode_characteristic.u,
                .access_cb = state_mode_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                0, /* No more characteristics in this service. */
            }
        },
    },
    {
        0, /* No more services. */
    },
};

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
    

    // uint8_t* color = (uint8_t*)dst;
    // rgb_t color_;
    // color_.red = color[1];
    // color_.green = color[0];
    // color_.blue = color[2];
    // ESP_LOGE("LED", "fill %02X %02X %02X", color_.red, color_.green, color_.blue);
    // led_strip_set_pixel(config_ctx->)
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
