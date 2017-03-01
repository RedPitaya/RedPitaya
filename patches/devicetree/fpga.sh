#!/bin/bash
mkdir /sys/kernel/config/device-tree/overlays/fpga
cat /opt/redpitaya/fpga.dtbo > /sys/kernel/config/device-tree/overlays/fpga/dtbo
sleep 1
#rmdir /sys/kernel/config/device-tree/overlays/fpga

