#!/bin/bash

cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg

/opt/redpitaya/sbin/getsysinfo.sh

# Here you can specify commands for autorun at system startup

