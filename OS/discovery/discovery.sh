#!/bin/sh
################
# Old Discovery
################

MAC=`cat /sys/class/net/eth0/address`
IP=`ip -o -4 addr show | awk -F '[ /]+' '/global/ {print $4}'`

PAYLOAD=payload={\"mac\":\"$MAC\",\"ips\":{\"eth0\":\"$IP\"}}

URL=http://discovery.redpitaya.com/update

curl -X POST -d "$PAYLOAD" $URL

################
# New Discovery
################

URL_PREFIX="http://account.staging.redpitaya.com/discovery.php"
LOGFILE="/var/log/discovery.log"

MAC_DEFAULT=$(cat /sys/class/net/eth0/address)
IP_LAN=$(ip -o -f inet addr show eth0 | awk '{print $4}' | cut -d"/" -f1)
IP_WAN=$(ip -o -f inet addr show wlan0 | awk '{print $4}' | cut -d"/" -f1)

if [ -n "$IP_LAN" ];then
    PAYLOAD="?mac=$MAC_DEFAULT&ip=$IP_LAN"
    if [ -n "$IP_WAN" ]; then
        PAYLOAD=$PAYLOAD"&ipwifi=$IP_WAN"
    fi
else
    PAYLOAD="?mac=$MAC_DEFAULT&ip=$IP_WAN"
fi

curl $URL_PREFIX$PAYLOAD >> $LOGFILE 2>&1
if [ $? -ne 0 ]
then
    echo "Discovery update failed!" >> $LOGFILE
else
    echo "Discovery update was successful." >> $LOGFILE
fi

