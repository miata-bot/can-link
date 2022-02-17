#!/bin/sh

fwup -c -f /etc/storage.conf | fwup -i - -a -t complete -d /tmp/lun0.img