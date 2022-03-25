#!/bin/sh

cd /sys/kernel/config/usb_gadget/g1
echo "" > UDC
sleep 1
rm -rf /sys/kernel/config/usb_gadget/g1
