#!/bin/bash

READ_MAC=$( fw_printenv | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
READ_HWREV=$( fw_printenv | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
DNA_1=$( monitor 0x40000008 | awk 'BEGIN {FS="x"}{print $2}') > /dev/null 2>&1
DNA_2=$( monitor 0x40000004 | awk 'BEGIN {FS="x"}{print $2}') > /dev/null 2>&1

echo { > /tmp/sysinfo.json
echo \"model\": \"$READ_HWREV\", >> /tmp/sysinfo.json
echo \"mac\": \"$READ_MAC\", >> /tmp/sysinfo.json
echo \"dna\": \"$DNA_1$DNA_2\", >> /tmp/sysinfo.json
echo \"ecosystem\": $(cat /opt/redpitaya/www/apps/info/info.json ), >> /tmp/sysinfo.json
echo \"linux\": \"$(cat /root/.version)\"  >> /tmp/sysinfo.json
echo } >> /tmp/sysinfo.json