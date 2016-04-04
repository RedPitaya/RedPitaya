#!/bin/bash

cd /tmp/build/ && killall nginx && rw && /bin/cp -fr * /opt/redpitaya && reboot
