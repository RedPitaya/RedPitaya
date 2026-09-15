#!/bin/bash
READ_MAC=""
READ_HWREV=""
DNA_1=""
DNA_2=""
C=10
FPGA_VER=$(profiles -f)
S_VER=$(profiles -i)
NAME=$(profiles -n)
IS_MOLDE_VALID=$(profiles -c)
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
echo \"name\": \"$NAME\", >> /tmp/sysinfo.json
echo \"is_slave\": \"$SLAVE\", >> /tmp/sysinfo.json
echo \"is_model_valid\": \"$IS_MOLDE_VALID\", >> /tmp/sysinfo.json
echo \"stem_ver\": \"$S_VER\", >> /tmp/sysinfo.json
echo \"mac\": \"$READ_MAC\", >> /tmp/sysinfo.json
echo \"dna\": \"$DNA_1$DNA_2\", >> /tmp/sysinfo.json
echo \"ecosystem\": $(cat /opt/redpitaya/www/apps/info/info.json ), >> /tmp/sysinfo.json
echo \"linux\": \"$(cat /root/.version)\",  >> /tmp/sysinfo.json
echo \"mem_size\":\"$(($(getconf _PHYS_PAGES) * $(getconf PAGE_SIZE) / (1024 * 1024)))\",  >> /tmp/sysinfo.json

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
