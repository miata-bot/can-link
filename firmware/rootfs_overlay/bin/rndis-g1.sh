#!/bin/sh

/bin/setup-gadget1.sh

cd /sys/kernel/config/usb_gadget/g1

mkdir -p functions/acm.usb0    # serial
mkdir -p functions/rndis.usb0  # network

mkdir -p configs/c.1
echo 250 > configs/c.1/MaxPower
ln -s functions/rndis.usb0 configs/c.1/
ln -s functions/acm.usb0   configs/c.1/

# OS descriptors
echo 1       > os_desc/use
echo 0xcd    > os_desc/b_vendor_code
echo MSFT100 > os_desc/qw_sign

echo RNDIS   > functions/rndis.usb0/os_desc/interface.rndis/compatible_id
echo 5162001 > functions/rndis.usb0/os_desc/interface.rndis/sub_compatible_id

ln -s configs/c.1 os_desc
echo musb-hdrc.0 > UDC
