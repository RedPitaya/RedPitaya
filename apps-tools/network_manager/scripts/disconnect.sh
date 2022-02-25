#!/bin/bash


RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh

RES=$?

$RP_PATH/remove_ap.sh

if [ $RES == 2 ]; then
        rw
	killall wpa_supplicant 2> /dev/null
        rm -f /opt/redpitaya/wpa_supplicant.conf
        ip link set dev wlan0 down
        sleep 2
        ip link set dev wlan0 up
        ro
        sleep 2
        exit $RES
fi

if [ $RES == 1 ]; then
	rw
	killall wpa_supplicant 2> /dev/null
	rm -f /opt/redpitaya/wpa_supplicant.conf
	iw dev wlan0 disconnect 2> /dev/null
        ip link set dev wlan0 down
        sleep 2
        ip link set dev wlan0 up
        ro
        sleep 2
        exit $RES
fi

