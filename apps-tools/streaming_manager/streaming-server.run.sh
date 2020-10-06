#!/bin/bash
LD_LIBRARY_PATH=/opt/redpitaya/lib
export LD_LIBRARY_PATH
./streaming-server -c /root/.streaming_config
