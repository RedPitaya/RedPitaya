#!/bin/bash
mkdir /sys/kernel/config/device-tree/overlays/amba_pl
cat amba_pl.dtbo > /sys/kernel/config/device-tree/overlays/amba_pl/dtbo
