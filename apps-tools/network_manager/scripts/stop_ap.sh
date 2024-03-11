#!/bin/bash

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts

rw
$RP_PATH/apmode_def_route.sh r
systemctl stop hostapd@wlan0.service
rm -f /opt/redpitaya/hostapd.conf
ro
kill -HUP $(pgrep nginx|head -1)
sleep 2
