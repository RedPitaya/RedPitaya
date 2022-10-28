#!/bin/sh

FPGAS=/opt/redpitaya/fpga

sleep 0.5s

rmdir /configfs/device-tree/overlays/Full 2> /dev/null
rm /tmp/loaded_fpga.inf 2> /dev/null

sleep 0.5s

/opt/redpitaya/bin/fpgautil -b $FPGAS/$1/fpga.bit.bin -o $FPGAS/$1/fpga.dtbo -n Full

if [[ "$?" = '0' ]]
then
    echo -n $1 > /tmp/loaded_fpga.inf
else
    rm /tmp/loaded_fpga.inf 2> /dev/null
fi
