#!/bin/bash

FPGAS=/opt/redpitaya/fpga

/opt/redpitaya/bin/fpgautil -b $FPGAS/$1/fpga.bit.bin -o $FPGAS/$1/fpga.dtbo -n Full

if [[ "$?" = '0' ]]
then
    echo -n $1 > /tmp/loaded_fpga.inf
else
    rm /tmp/loaded_fpga.inf 2> /dev/null
fi