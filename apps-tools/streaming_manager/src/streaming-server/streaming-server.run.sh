#!/bin/bash
LD_LIBRARY_PATH=/opt/redpitaya/lib
export LD_LIBRARY_PATH
/opt/redpitaya/sbin/overlay.sh stream_app
sleep 1
./streaming-server -v
