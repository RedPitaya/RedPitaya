#!/bin/bash
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
/opt/redpitaya/bin/generate 1 0.35 25678901 sine
/opt/redpitaya/bin/generate 2 0.35 29012345 sine
