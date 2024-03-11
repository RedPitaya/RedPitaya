#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh

RES=$?

RET=''

if [ $RES == 2 ]; then
        RET="" #TODO  #$(/sbin/iwconfig wlan0 2> /dev/null | /bin/grep SSID | /usr/bin/gawk -F\" '{print $2}')
fi

if [ $RES == 1 ]; then
        RET=$(iw dev wlan0 info | gawk '/ssid/{print $2}')
fi

if [ "$RET" != "" ]; then
	echo "$RET"
	exit $RES
fi

echo ''
exit $RES

