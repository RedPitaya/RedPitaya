#!/bin/bash
LD_LIBRARY_PATH=/opt/redpitaya/lib
export LD_LIBRARY_PATH
cat /opt/redpitaya/fpga/fpga_streaming.bit  > /dev/xdevcfg
/opt/redpitaya/sbin/mkoverlay.sh stream_app
./streaming-server
