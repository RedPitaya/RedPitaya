#!/bin/sh
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
/opt/redpitaya/bin/discovery

/opt/redpitaya/sbin/sockproc /tmp/shell.sock