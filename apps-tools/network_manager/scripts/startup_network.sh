#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts
WPA_SUP=/opt/redpitaya/wpa_supplicant.conf
HOST=/opt/redpitaya/hostapd.conf

if [[ -f "$WPA_SUP" && -f "$HOST" ]]; then
    echo "Wrong configuration"
    exit -1
fi

$RP_PATH/get_connected_wifi.sh
RES=$?

if [ -f "$WPA_SUP" ]; then

    if [ $RES == 2 ]; then
        SSID=$(cat /opt/redpitaya/wpa_supplicant.conf | gawk -F\" '/ssid/{print $2}')
    	/sbin/iwconfig wlan0 mode Managed essid $SSID
	/sbin/wpa_supplicant -B -i wlan0 -c /opt/redpitaya/wpa_supplicant.conf -D wext
    fi

    if [ $RES == 1 ]; then
	    /sbin/wpa_supplicant -B -c/etc/wpa_supplicant/wpa_supplicant.conf -iwlan0
    fi

fi
