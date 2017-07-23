#!/bin/bash
mkdir /sys/kernel/config/device-tree/overlays/amba_pl
cat amba_pl.dtbo > /sys/kernel/config/device-tree/overlays/amba_pl/dtbo
# wait a bit for the kernel to process the overlay,
# before attempts are made to use the new drivers
sleep 0.5s
