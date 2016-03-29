#!/bin/bash

cd /tmp/build/ecosystem-0 && killall nginx && rw && rm -rf /opt/redpitaya/ && cd /tmp/build/ecosystem-0 && cp -fr * /opt/ && reboot
