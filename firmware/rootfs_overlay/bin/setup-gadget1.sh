#!/bin/sh

cd /sys/kernel/config/usb_gadget/
mkdir g1 && cd g1

echo 0x1d6b > idVendor  # Linux Foundation
echo 0x0104 > idProduct # Multifunction Composite Gadget
echo 0x0100 > bcdDevice # v1.0.0
echo 0x0200 > bcdUSB    # USB 2.0

echo 0xEF > bDeviceClass
echo 0x02 > bDeviceSubClass
echo 0x01 > bDeviceProtocol

mkdir -p strings/0x409
serial=$(boardid)
echo $serial > strings/0x409/serialnumber
echo "Cone"        > strings/0x409/manufacturer
echo "CAN Underglow Controller 1"   > strings/0x409/product
