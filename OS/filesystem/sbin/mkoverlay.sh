#!/bin/bash

FPGAS=/opt/redpitaya/fpga
MODEL=$(/opt/redpitaya/bin/monitor -f)
if [ "$?" = "0" ]
then

/opt/redpitaya/bin/fpgautil -b $FPGAS/$MODEL/$1/fpga.bit.bin -o $FPGAS/$MODEL/$1/fpga.dtbo -n Full

if [ "$?" = '0' ]
then
    echo -n $1 > /tmp/loaded_fpga.inf
    exit 0
else
    rm /tmp/loaded_fpga.inf 2> /dev/null
    exit 1
fi
fi
exit 1
