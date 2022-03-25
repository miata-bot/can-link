#!/bin/sh
/bin/setup-gadget2.sh

cd /sys/kernel/config/usb_gadget/g2

# create configs
mkdir configs/c.1

# configure them with attributes if needed
echo 120 > configs/c.1/MaxPower

# ensure function is loaded
modprobe usb_f_mass_storage

# create the function (name must match a usb_f_<name> module such as 'acm')
mkdir -p functions/mass_storage.0

# associate with partitions
mkdir -p functions/mass_storage.0/lun.0
echo /dev/mmcblk0p4 > functions/mass_storage.0/lun.0/file

# associate function with config
ln -s functions/mass_storage.0 configs/c.1

echo musb-hdrc.0 > UDC
