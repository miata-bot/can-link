#include <led_strip.h>
#include "gatt_srv.h"

extern led_strip_t strip_channel_1;
extern led_strip_t strip_channel_2;

static uint32_t addressable_led_channel_1_color = 0;
static uint32_t addressable_led_channel_2_color = 0;

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &led_control_service.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {   {
                /** Characteristic: Addressable LED Channel 1 */
                .uuid = &addressable_led_channel_1_characteristic.u,
                .access_cb = addressable_led_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            }, {
                /** Characteristic: Addressable LED Channel 2 */
                .uuid = &addressable_led_channel_2_characteristic.u,
                .access_cb = addressable_led_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            }, {
                0, /* No more characteristics in this service. */
            }
        },
    },

    {
        .type = BLE_GATT_SVC_TYPE_SECONDARY,
        .uuid = &config_service.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {   {
                /** Characteristic:  */
                .uuid = &asdf.u,
                .access_cb = asdf,
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

    uint8_t* color = (uint8_t*)dst;
    rgb_t color_;
    color_.red = color[1];
    color_.green = color[0];
    color_.blue = color[2];
    led_strip_fill(strip, 0, strip->length, color_);
    led_strip_flush(strip);
    return 0;
}

static int
addressable_led_access(uint16_t conn_handle, uint16_t attr_handle,
                       struct ble_gatt_access_ctxt *ctxt,
                       void *arg)
{
    const ble_uuid_t *uuid = ctxt->chr->uuid;
    int rc;

    // channel 1
    if (ble_uuid_cmp(uuid, &addressable_led_channel_1_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &addressable_led_channel_1_color,
                                sizeof addressable_led_channel_1_color);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = addressable_led_handle_write(&strip_channel_1, ctxt->om,
                                              sizeof addressable_led_channel_1_color,
                                              sizeof addressable_led_channel_1_color,
                                              &addressable_led_channel_1_color, NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    // channel 2
    if (ble_uuid_cmp(uuid, &addressable_led_channel_2_characteristic.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = os_mbuf_append(ctxt->om, &addressable_led_channel_2_color,
                                sizeof addressable_led_channel_2_color);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = addressable_led_handle_write(&strip_channel_2, ctxt->om,
                                              sizeof addressable_led_channel_2_color,
                                              sizeof addressable_led_channel_2_color,
                                              &addressable_led_channel_2_color, NULL);
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
gatt_svr_init(void)
{
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

    return 0;
}
