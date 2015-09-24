#!/bin/sh

URL="http://account.redpitaya.com/discovery.php"
LOGFILE="/var/log/discovery.log"

MAC_LAN=$(cat /sys/class/net/eth0/address)
IP_LAN=$(ip -o -f inet addr show eth0  | awk '{print $4}' | cut -d"/" -f1)
IP_WAN=$(ip -o -f inet addr show wlan0 | awk '{print $4}' | cut -d"/" -f1)

if [ -n "$IP_LAN" ];then
    PAYLOAD="?mac=$MAC_LAN&ip=$IP_LAN"
    if [ -n "$IP_WAN" ]; then
        PAYLOAD=$PAYLOAD"&ipwifi=$IP_WAN"
    fi
else
    PAYLOAD="?mac=$MAC_LAN&ip=$IP_WAN"
fi

curl $URL$PAYLOAD >> $LOGFILE 2>&1

if [ $? -ne 0 ]
then
    echo "Discovery update failed!"
else
    echo "Discovery update was successful."
fi
