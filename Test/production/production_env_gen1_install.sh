#!/bin/bash
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
/opt/redpitaya/bin/generate 1 0.35 25678901 sine
echo "generator started"
