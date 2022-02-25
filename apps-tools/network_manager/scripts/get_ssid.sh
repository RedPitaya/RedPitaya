#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh

RES=$?

if [ $RES == 2 ]; then
	echo $(iwconfig wlan0 2> /dev/null | grep SSID | gawk -F\" '{print $2}')
	exit $RES
fi

if [ $RES == 1 ]; then
        echo $(iw dev wlan0 link | grep SSID | gawk -F: '$1 == "\tSSID" {print $2}')
	exit $RES
fi

echo ''
exit $RES
