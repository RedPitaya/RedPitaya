#!/bin/sh

MAC=`cat /sys/class/net/eth0/address`
IP=`ip -o -4 addr show | awk -F '[ /]+' '/global/ {print $4}'`

PAYLOAD=payload={\"mac\":\"$MAC\",\"ips\":{\"eth0\":\"$IP\"}}

URL=http://discovery.redpitaya.com/update

curl -X POST -d '$PAYLOAD' $URL
