#!/bin/bash

systemctl stop redpitaya_updater.service 
/bin/cp -f /opt/redpitaya/bin/updater /tmp
systemctl start redpitaya_updater.service 