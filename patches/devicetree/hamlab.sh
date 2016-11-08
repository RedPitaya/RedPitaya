#!/bin/bash

# apply HAMLAB device tree overlay
mkdir /sys/kernel/config/device-tree/overlays/hamlab
cat /boot/hamlab.dtbo >/sys/kernel/config/device-tree/overlays/hamlab/dtbo

# just in case if pulldown resistor is not soldered ok
# set to 0
i2cset -y 7 0x20 2 0
i2cset -y 7 0x20 3 0

# set for output
i2cset -y 7 0x20 6 0
i2cset -y 7 0x20 7 0

# Fan control contiguration
# set 22.5kHz pwm
i2cset -y -r 2 0x2e 0x74 0x70
# insert fan control kernel driver
echo adt7470 0x2e > /sys/bus/i2c/devices/i2c-2/new_device
