#!/bin/bash

# common directories
FPGAS=/opt/redpitaya/fpga
OVERLAYS=/sys/kernel/config/device-tree/overlays

# first argument is overlay name
OVERLAY=$1

# first load the fpga, then the overlay
mkdir $OVERLAYS/$OVERLAY
cat $FPGAS/$OVERLAY/fpga.dtbo > $OVERLAYS/$OVERLAY/dtbo

# wait a bit for the kernel to process the overlay,
# before attempts are made to use the new drivers
sleep 0.5s
