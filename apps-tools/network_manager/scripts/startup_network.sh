#!/bin/bash
#
# Manual bring-up from a stored configuration. NOT on the boot path any more:
# netstart.service is gone and wpa_supplicant@wlan0.service owns connecting at
# boot (see RedPitaya/ubuntu, debian/wireless_tool.sh). Kept as a hand tool.

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

    ip link set dev wlan0 up 2>/dev/null

    if [ $RES == 1 ]; then
        /sbin/wpa_supplicant -B -i wlan0 -c "$WPA_SUP" -D nl80211
    fi

fi
