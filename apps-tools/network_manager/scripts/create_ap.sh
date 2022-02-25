#!/bin/bash

SSID=$1
PASS=$2

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh

RES=$?

if [ $RES == 2 ]; then
	echo "NOT SUPPORTED"
fi

if [ $RES == 1 ]; then
    $RP_PATH/disconnect.sh
	rw
	CONF_F=/opt/redpitaya/hostapd.conf
	echo "interface=wlan0" > $CONF_F
    echo "ssid=$SSID" >> $CONF_F
	echo "driver=nl80211" >> $CONF_F
	echo "hw_mode=g" >> $CONF_F
	echo "channel=6" >> $CONF_F
	echo "macaddr_acl=0" >> $CONF_F
	echo "auth_algs=1" >> $CONF_F
    echo "ignore_broadcast_ssid=0" >> $CONF_F
    echo "wpa=2" >> $CONF_F
    echo "wpa_passphrase=$PASS" >> $CONF_F
    echo "wpa_key_mgmt=WPA-PSK" >> $CONF_F
	echo "wpa_pairwise=CCMP" >> $CONF_F
	echo "rsn_pairwise=CCMP" >> $CONF_F
  	systemctl restart hostapd@wlan0.service
    sleep 1
    systemctl restart wireless-mode-ap.service
    sleep 1
    systemctl restart systemd-networkd.service
    sleep 2
    ro
	$RP_PATH/apmode_def_route.sh a
	exit $RES
fi

exit 0
