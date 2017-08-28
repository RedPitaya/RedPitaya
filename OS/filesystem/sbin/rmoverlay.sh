#!/bin/bash

# common directories
FPGAS=/opt/redpitaya/fpga
OVERLAYS=/sys/kernel/config/device-tree/overlays

# first argument is overlay name
OVERLAY=$1

# first remove existing overlays, there is no way to unload the FPGA
rmdir $OVERLAYS/*
