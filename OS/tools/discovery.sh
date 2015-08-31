#!/bin/sh

URL_PREFIX="http://account.staging1.redpitaya.com/discovery.php"
LOGFILE="/var/log/discovery.log"

MAC_DEFAULT=$(cat /sys/class/net/eth0/address)
MAC_UPPER_CASE=$(echo "$MAC_DEFAULT" | awk '{print toupper($0)}')
IP_LAN=$(ip -o -f inet addr show eth0  | awk '{print $4}' | cut -d"/" -f1)
IP_WAN=$(ip -o -f inet addr show wlan0 | awk '{print $4}' | cut -d"/" -f1)

if [ -n "$IP_LAN" ];then
    PAYLOAD="?mac=$MAC_DEFAULT&ip=$IP_LAN"
    PAYLOAD_UPPER="?mac=$MAC_DEFAULT&ip=$IP_LAN"
    if [ -n "$IP_WAN" ]; then
        PAYLOAD=$PAYLOAD"&ipwifi=$IP_WAN"
        PAYLOAD_UPPER=$PAYLOAD_UPPER"&ipwifi=$IP_WAN"
    fi
else
    PAYLOAD="?mac=$MAC_DEFAULT&ip=$IP_WAN"
    PAYLOAD_UPPER="?mac=$MAC_DEFAULT&ip=$IP_WAN"
fi

curl $URL_PREFIX$PAYLOAD >> $LOGFILE 2>&1
curl $URL_PREFIX$PAYLOAD_UPPER >> $LOGFILE 2>&1

if [ $? -ne 0 ]
then
    echo "Discovery update failed!" >> $LOGFILE
else
    echo "Discovery update was successful." >> $LOGFILE
fi
