#!/bin/bash

SSID=$1
PASS=$2

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh

RES=$?

if [ $RES == 2 ]; then
	$RP_PATH/disconnect.sh
	rw
	wpa_passphrase $SSID $PASS > /opt/redpitaya/wpa_supplicant.conf
	iwconfig wlan0 mode Managed essid $SSID
	wpa_supplicant -B -i wlan0 -c /opt/redpitaya/wpa_supplicant.conf -D wext
	sleep 1
	systemctl restart wireless-mode-client.service
	sleep 1
	systemctl restart systemd-networkd.service
	ro
	sleep 2
	exit $RES
fi

if [ $RES == 1 ]; then
	$RP_PATH/disconnect.sh
	rw
	wpa_passphrase $SSID $PASS > /opt/redpitaya/wpa_supplicant.conf
	wpa_supplicant -B -c/etc/wpa_supplicant/wpa_supplicant.conf -iwlan0
	sleep 1
	systemctl restart wireless-mode-client.service
	sleep 1
	systemctl restart systemd-networkd.service
	ro
	sleep 2
	exit $RES
fi

exit -1
