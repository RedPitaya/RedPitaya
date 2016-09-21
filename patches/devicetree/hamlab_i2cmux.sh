#!/bin/bash
mkdir /sys/kernel/config/device-tree/overlays/i2cmux
cat /boot/hamlab_i2cmux.dtbo >/sys/kernel/config/device-tree/overlays/i2cmux/dtbo

i2cset -y 8 0x20 6 0
i2cset -y 8 0x20 7 0
i2cset -y 8 0x21 6 0
i2cset -y 8 0x21 7 0
i2cset -y 8 0x20 2 0
i2cset -y 8 0x20 3 0
i2cset -y 8 0x21 2 0
i2cset -y 8 0x21 3 0

echo adt7470 0x2e > /sys/bus/i2c/devices/i2c-2/new_device
