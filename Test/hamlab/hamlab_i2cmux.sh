#!/bin/bash
mkdir /sys/kernel/config/device-tree/overlays/i2cmux
cat /root/hamlab_i2cmux.dtbo >/sys/kernel/config/device-tree/overlays/i2cmux
