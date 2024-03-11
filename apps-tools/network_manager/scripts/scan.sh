#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh

RES=$?

if [ $RES == 2 ]; then
	iwlist wlan0 scan | gawk -f /opt/redpitaya/www/apps/network_manager/scripts/iwlist_scan.awk
        exit $RES
fi

iw dev wlan0 scan | gawk -f /opt/redpitaya/www/apps/network_manager/scripts/iw_scan.awk
exit $RES

