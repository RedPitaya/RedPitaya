#!/bin/bash

cd /tmp/build/ && killall nginx && rw && rm -rf /opt/redpitaya/* && cp -fr * /opt/redpitaya && reboot
