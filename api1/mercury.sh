#!/bin/sh

FPGAS=/opt/redpitaya/fpga
OVERLAYS=/sys/kernel/config/device-tree/overlays
OVERLAY=mercury

cat $FPGAS/$OVERLAY/fpga.bit > /dev/xdevcfg
rmdir $OVERLAYS/*
mkdir $OVERLAYS/$OVERLAY
cat $FPGAS/$OVERLAY/fpga.dtbo > $OVERLAYS/$OVERLAY/dtbo
