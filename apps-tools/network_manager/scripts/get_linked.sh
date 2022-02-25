#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh
echo -n $?

SSID=$($RP_PATH/get_ssid.sh)

if [ "$SSID" != "" ]; then
	echo 1
	exit 1
fi

SSID=$($RP_PATH/get_ssid_ap.sh)

if [ "$SSID" != "" ]; then
	echo 2
	exit 2
fi

echo 0
exit 0

