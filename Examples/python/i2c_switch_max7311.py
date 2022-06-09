#!/usr/bin/python

import sys
import time
from struct import *
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

# working with RP 250-12 v1.2. For RP version 1.1 need replace dev address to 32

rp_s.tx_txt('I2C:DEV33 "/dev/i2c-0"')
print("Init I2C")

rp_s.tx_txt('I2C:FMODE ON')
print("Set force mode")

#  Swich AC_DC for In 1

print("Turn on AC/DC ch1 & ch2")

value = 0x55;
rp_s.tx_txt('I2C:S:W2 ' + str(value)) # write to i2c
print("Write value for reg 0x2",value)

time.sleep(1)

value = (value & ~0x0F);
rp_s.tx_txt('I2C:S:W2 ' + str(value)) # write to i2c
print("Write value for reg 0x2",value)

time.sleep(3)

print("Turn off AC/DC ch1 & ch2")

value = 0xAA;
rp_s.tx_txt('I2C:S:W2 ' + str(value)) # write to i2c
print("Write value for reg 0x2",value)

time.sleep(1)

value = (value & ~0x0F);
rp_s.tx_txt('I2C:S:W2 ' + str(value)) # write to i2c
print("Write value for reg 0x2",value)

rp_s.tx_txt('I2C:S:R2')
value = int(rp_s.rx_txt())
print("Read value for reg 0x2",value)

