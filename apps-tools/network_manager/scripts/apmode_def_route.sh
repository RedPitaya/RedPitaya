#!/bin/bash
IP=$(ip -4 -o addr list wlan0 | awk '{print $4}' | cut -d "/" -f 1 | tail -n1)

if [ "$IP" == "" ]; then
	exit -1
fi

if [[ "$1" == "a" ]]
then
    /bin/ip route add default via $IP src $IP metric 1
    /bin/ip route add 0.0.0.0/1 via wlan0 metric 1
    /bin/ip route add 128.0.0.0/1 via wlan0 metric 1
fi

if [[ "$1" == "r" ]]
then
    /bin/ip route delete default via $IP src $IP metric 1
fi

