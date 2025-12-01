#!/bin/sh

# Check min 50MB
MIN_TRH=52428800
FREE=$(parted /dev/mmcblk0 unit B print free | grep 'Free Space' | tail -n1 | awk '{print substr($3, 1, length($3)-1)}')

if [ "$FREE" -gt "$MIN_TRH" ]; then
    /opt/redpitaya/sbin/resize.sh
    reboot
fi