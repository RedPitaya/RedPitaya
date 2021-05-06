#!/bin/bash

cd /tmp/build/ && killall nginx && rw && /bin/rm -rf /opt/redpitaya/* && /bin/cp -rf * /opt/redpitaya && reboot
