#!/bin/bash
READ_MAC=""
READ_HWREV=""
DNA_1=""
DNA_2=""
C=10
FPGA_VER=$(monitor -f)
IS_512_BOOT=$(cmp /opt/redpitaya/boot.bin /opt/redpitaya/uboot/boot_512Mb_ram.bin)
SLAVE=$(cat /opt/redpitaya/bin/.streaming_mode 2> /dev/null)

STATE=$(cat /tmp/loaded_fpga.inf 2> /dev/null)
if [ "$STATE" = "v0.94" ]
then

if [ "$SLAVE" = "slave mode" ]
then
    while [[ "$READ_MAC" == "" || "$READ_HWREV" == "" ]] && [[ $C -ge  0 ]]
    do
        READ_MAC=$( fw_printenv | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
        READ_HWREV=$( fw_printenv | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
        C=$[$C - 1]
        sleep 1
    done
    READ_HWREV="$READ_HWREV SLAVE"
    DNA_1="-"
else
    while [[ "$READ_MAC" == "" || "$READ_HWREV" == "" || "$DNA_1" == "" || "$DNA_2" == "" ]] && [[ $C -ge  0 ]]
    do
        READ_MAC=$( fw_printenv | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
        READ_HWREV=$( fw_printenv | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
        DNA_1=$( monitor 0x40000008 | awk 'BEGIN {FS="x"}{print $2}') > /dev/null 2>&1
        DNA_2=$( monitor 0x40000004 | awk 'BEGIN {FS="x"}{print $2}') > /dev/null 2>&1
        C=$[$C - 1]
        sleep 1
    done
fi

echo { > /tmp/sysinfo.json
echo \"model\": \"$READ_HWREV\", >> /tmp/sysinfo.json
echo \"mac\": \"$READ_MAC\", >> /tmp/sysinfo.json
echo \"dna\": \"$DNA_1$DNA_2\", >> /tmp/sysinfo.json
echo \"ecosystem\": $(cat /opt/redpitaya/www/apps/info/info.json ), >> /tmp/sysinfo.json
echo \"linux\": \"$(cat /root/.version)\",  >> /tmp/sysinfo.json
echo \"mem_size\":\"$(($(getconf _PHYS_PAGES) * $(getconf PAGE_SIZE) / (1024 * 1024)))\",  >> /tmp/sysinfo.json

if [ "$FPGA_VER" == "z20_250" ]; then
echo \"mem_upgrade\":\"1\",  >> /tmp/sysinfo.json
else
echo \"mem_upgrade\":\"0\",  >> /tmp/sysinfo.json
fi

if [ -z "$IS_512_BOOT" ]; then
echo \"boot_512\":\"1\",  >> /tmp/sysinfo.json
else
echo \"boot_512\":\"0\",  >> /tmp/sysinfo.json
fi


echo \"linux\": \"$(cat /root/.version)\",  >> /tmp/sysinfo.json

echo \"fpga\":{  >> /tmp/sysinfo.json

FPGALIST_EX=0
for f in /opt/redpitaya/fpga/$FPGA_VER/*; do
    if [ -d "$f" ]; then
        DIR_NAME=$(basename $f)
        COMMIT=$(awk 'NR==2 {print substr($2,0,9)}' $f/git_info.txt)
        echo \"$DIR_NAME\":\"$COMMIT\", >> /tmp/sysinfo.json
        FPGALIST_EX=1
    fi
done
# remove last character in file
if [ $FPGALIST_EX == 1 ]
then
sed -i '$ s/.$//' /tmp/sysinfo.json
fi
echo }  >> /tmp/sysinfo.json
echo } >> /tmp/sysinfo.json

fi
