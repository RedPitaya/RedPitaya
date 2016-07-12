#!/bin/bash
rw
cp /opt/redpitaya/lib/librp2.so /lib/
(crontab -l 2>/dev/null; echo "@reboot /opt/redpitaya/bin/laboardtest") | crontab -
