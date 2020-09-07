#!/bin/bash

echo "Setting DEFAULT environment parameters on the external EEPROM...."
cat /opt/redpitaya/environment_parameters.txt > /sys/devices/soc0/amba/e0004000.i2c/i2c-0/0-0050/eeprom

