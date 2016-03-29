#!/bin/bash

cd /tmp/build/ecosystem* && killall nginx && rw && rm -rf /opt/redpitaya/* && cp -fr * /opt/redpitaya && reboot
