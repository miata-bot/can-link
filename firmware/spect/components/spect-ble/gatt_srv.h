#ifndef CONEPROJ_GATT_SRV_H
#define CONEPROJ_GATT_SRV_H

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

#include "bleprph.h"

/* b7528e5d-1f1e-4ee6-a3a2-2b3f45a7ca8e */
static const ble_uuid128_t state_service =
  BLE_UUID128_INIT(0x8E,0xCA,0xA7,0x45,0x3F,0x2B,0xA2,0xA3,
                   0xE6,0x4E,0x1E,0x1F,0x5D,0x8E,0x52,0xB7);

/* 733ec5d8-5775-434d-852c-a4e7cb282a10 */
static const ble_uuid128_t state_mode_characteristic =
  BLE_UUID128_INIT(0x10,0x2A,0x28,0xCB,0xE7,0xA4,0x2C,0x85,
                   0x4D,0x43,0x75,0x57,0xD8,0xC5,0x3E,0x73);

static int state_mode_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);
          
/* 59462f12-9543-9999-12c8-58b459a2712d */
static const ble_uuid128_t led_control_service =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* 5c3a659e-897e-45e1-b016-007107c96df7 */
static const ble_uuid128_t addressable_led_channel_1_characteristic =
    BLE_UUID128_INIT(0xf7, 0x6d, 0xc9, 0x07, 0x71, 0x00, 0x16, 0xb0,
                     0xe1, 0x45, 0x7e, 0x89, 0x9e, 0x65, 0x3a, 0x5c);

/* 5c3a659e-897e-45e1-b016-007107c96df8 */
static const ble_uuid128_t addressable_led_channel_2_characteristic =
    BLE_UUID128_INIT(0xf8, 0x6d, 0xc9, 0x07, 0x71, 0x00, 0x16, 0xb0,
                     0xe1, 0x45, 0x7e, 0x89, 0x9e, 0x65, 0x3a, 0x5c);

/* 7db13f26-4d8e-4dd0-b810-66e5c13be8b9 */
static const ble_uuid128_t rgb_led_channel_1_characteristic =
  BLE_UUID128_INIT(0xB9,0xE8,0x3B,0xC1,0xE5,0x66,0x10,0xB8,
                   0xD0,0x4D,0x8E,0x4D,0x26,0x3F,0xB1,0x7D);

/* 4745abd8-1f15-4059-8985-d74c550ddfd1 */
static const ble_uuid128_t rgb_led_channel_2_characteristic =
  BLE_UUID128_INIT(0xD1,0xDF,0xD,0x55,0x4C,0xD7,0x85,0x89,
                   0x59,0x40,0x15,0x1F,0xD8,0xAB,0x45,0x47);

static int led_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);

/* cb80f936-a431-4170-a2d4-b937b883c2c5 */
static const ble_uuid128_t config_service =
    BLE_UUID128_INIT(0xC5,0xC2,0x83,0xB8,0x37,0xB9,0xD4,0xA2,
                     0x70,0x41,0x31,0xA4,0x36,0xF9,0x80,0xCB);

/* 45015d71-b511-4dd7-8aca-73afb527585d */
static const ble_uuid128_t config_strip_length_characteristic =
  BLE_UUID128_INIT(0x5D,0x58,0x27,0xB5,0xAF,0x73,0xCA,0x8A,
                   0xD7,0x4D,0x11,0xB5,0x71,0x5D,0x1,0x45);

static int config_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);
#endif