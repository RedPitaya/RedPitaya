#!/bin/sh

show_help() {
    echo "Usage: $0 <fpga_name> [custom_fpga] [custom_devicetree] [overlay_name]"
    echo ""
    echo "Load FPGA bitstream and device tree overlay"
    echo ""
    echo "Parameters:"
    echo "  <fpga_name>        - Name of FPGA configuration from /opt/redpitaya/fpga/\$MODEL/"
    echo "  [custom_fpga]      - Custom FPGA bitstream path (optional)"
    echo "  [custom_devicetree]- Custom device tree overlay path (optional)"
    echo "  [overlay_name]     - Custom overlay region name (optional, default: Full)"
    echo ""
    echo "Examples:"
    echo "  $0 mercury           - Load default Mercury FPGA"
    echo "  $0 oscillator /path/to/custom.bit.bin - Load custom FPGA bitstream"
    echo "  $0 sdr /path/to/custom.bit.bin /path/to/custom.dtbo - Load custom FPGA and device tree"
    echo "  $0 transmitter /path/to/fpga.bit.bin /path/to/fpga.dtbo CustomRegion - Load with custom overlay name"
    echo ""
    echo "Available FPGA configurations:"
    MODEL=$(/opt/redpitaya/bin/profiles -f 2>/dev/null)
    if [ "$?" = "0" ] && [ -d "/opt/redpitaya/fpga/$MODEL" ]; then
        for dir in /opt/redpitaya/fpga/$MODEL/*; do
            if [ -d "$dir" ] && [ -f "$dir/fpga.bit.bin" ]; then
                echo "  - $(basename "$dir")"
            fi
        done
    else
        echo "  (Unable to detect available configurations)"
    fi
}

if [ $# -eq 0 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    show_help
    exit 0
fi

CUSTOMDEVICETREE=/opt/$1/fpga.dtbo
CUSTOMFPGA=/opt/$1/fpga.bit.bin

FPGAS=/opt/redpitaya/fpga
MODEL=$(/opt/redpitaya/bin/profiles -f)

if [ "$?" = "0" ]
then
#sleep 0.5s

FPGA_REGION=Full
if [ "$#" -gt "3" ]
then
    FPGA_REGION=$4
    if [ "$FPGA_REGION" != "Led" ]; then
        rmdir /configfs/device-tree/overlays/$FPGA_REGION 2> /dev/null
    else
        echo "The overlay cannot use the name Led. This name is reserved for ecosystem purposes."
        exit 1
    fi
else
    for f in /configfs/device-tree/overlays/*; do
        # remove all existing overlay regions except for the Led directory, which is used by the system.
        if [ -d "$f" ] && [ "$(basename "$f")" != "Led" ]; then
            rmdir $f 2> /dev/null
        fi
    done
fi

rm -f /tmp/update_fpga.txt 2> /dev/null
rm -f /tmp/loaded_fpga.inf 2> /dev/null

#sleep 0.5s
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
