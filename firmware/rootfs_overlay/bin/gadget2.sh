#!/bin/sh

/bin/setup-gadget2.sh

cd /sys/kernel/config/usb_gadget/g1

mkdir -p functions/mass_storage.0/lun.0
echo /tmp/lun0.img > functions/mass_storage.0/lun.0/file
# echo 1 > functions/mass_storage.0/lun.0/luns
echo 1 > functions/mass_storage.0/lun.0/removable

mkdir -p configs/c.1

echo 500 > configs/c.1/MaxPower
ln -s functions/mass_storage.0 configs/c.1

ls musb-hdrc.0 > UDC
