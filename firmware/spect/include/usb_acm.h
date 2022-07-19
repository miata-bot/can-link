#ifndef CONEPROJ_USB_ACM_H
#define CONEPROJ_USB_ACM_H

#include <esp_log.h>
#include <esp_system.h>

#include "tinyusb.h"
#include "tusb_cdc_acm.h"

void acm_init();

#endif