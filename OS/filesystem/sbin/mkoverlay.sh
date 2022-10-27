#!/bin/bash

FPGAS=/opt/redpitaya/fpga

/opt/redpitaya/bin/fpgautil -b $FPGAS/$1/fpga.bit.bin -o $FPGAS/$1/fpga.dtbo -n Full
echo -n $1 > /tmp/loaded_fpga.inf