#!/bin/sh

CUSTOMDEVICETREE=/opt/$1/fpga.dtbo
CUSTOMFPGA=/opt/$1/fpga.bit.bin

FPGAS=/opt/redpitaya/fpga
MODEL=$(/opt/redpitaya/bin/monitor -f)

if [ "$?" = "0" ]
then
sleep 0.5s

FPGA_REGION=Full
if [ "$#" -gt "3" ]
then
    FPGA_REGION=$4
    rmdir /configfs/device-tree/overlays/$FPGA_REGION 2> /dev/null
else
    for f in /configfs/device-tree/overlays/*; do
        # remove all existing overlay regions
        if [ -d "$f" ]
        then
            rmdir $f 2> /dev/null
        fi
    done
fi

rm -f /tmp/update_fpga.txt 2> /dev/null
rm -f /tmp/loaded_fpga.inf 2> /dev/null

sleep 0.5s
FPGA_INF=$1
DEVICETREETOINSTALL=$FPGAS/$MODEL/$1/fpga.dtbo
FPGATOINSTALL=$FPGAS/$MODEL/$1/fpga.bit.bin
if [ "$#" -gt "1" ]
then
    FPGA_INF=$1_$2
    FPGATOINSTALL=$CUSTOMFPGA
fi

if [ "$#" -gt "2" ]
then
    FPGA_INF=$1_$2_$3
    DEVICETREETOINSTALL=$CUSTOMDEVICETREE
fi

MD5SUM=$(md5sum $FPGATOINSTALL)

if [ "$#" -eq "1" ]
then
    echo -n "Commit "
    awk 'NR==2 {print $2}' $FPGAS/$MODEL/$1/git_info.txt
else
    echo "Custom FPGA md5sum: $MD5SUM"
fi

/opt/redpitaya/bin/fpgautil -b $FPGATOINSTALL -o $DEVICETREETOINSTALL -n $FPGA_REGION > /tmp/update_fpga.txt 2>&1

if [ "$?" = "0" ]
then
    echo "FPGA md5sum: $MD5SUM" >> /tmp/update_fpga.txt
    date >> /tmp/update_fpga.txt
    echo -n $FPGA_INF > /tmp/loaded_fpga.inf
    exit 0
else
    rm -f /tmp/update_fpga.txt 2> /dev/null
    rm -f /tmp/loaded_fpga.inf 2> /dev/null
    exit 1
fi
fi
exit 1
