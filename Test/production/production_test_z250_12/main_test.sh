#!/bin/bash

echo "START MAIN TEST"

echo "LOAD FPGA IMAGE" 
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2
echo "FPGA LOADED SUCCESSFULLY"


echo "END MAIN TEST"
