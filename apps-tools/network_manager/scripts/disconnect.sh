#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts
WPA_SUP=/opt/redpitaya/wpa_supplicant.conf

$RP_PATH/get_connected_wifi.sh
RES=$?

$RP_PATH/remove_ap.sh

if [ "$RES" == "0" ]; then
    exit 0
fi

rw
killall wpa_supplicant 2> /dev/null
rm -f "$WPA_SUP"

iw dev wlan0 disconnect 2> /dev/null

ip link set dev wlan0 down
sleep 2
ip link set dev wlan0 up
ro
sleep 2
exit $RES
