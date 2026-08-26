#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

$RP_PATH/get_connected_wifi.sh
RES=$?

iw dev wlan0 scan | gawk -f $RP_PATH/iw_scan.awk
exit $RES
