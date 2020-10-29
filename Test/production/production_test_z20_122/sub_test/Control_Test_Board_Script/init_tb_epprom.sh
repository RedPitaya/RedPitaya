#!/bin/bash
FILE="$(mktemp)"
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/0-0050/eeprom > /sys/devices/soc0/amba/e0004000.i2c/i2c-0/0-0051/eeprom
echo /sys/bus/i2c/devices/0-0051/eeprom 0x1800 0x0400 > $FILE
fw_setenv -c $FILE test_label I2C_test
fw_printenv -c $FILE
rm -f $FILE
