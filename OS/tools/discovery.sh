#!/bin/sh
rm -f /tmp/shell.sock
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
/opt/redpitaya/bin/discovery
