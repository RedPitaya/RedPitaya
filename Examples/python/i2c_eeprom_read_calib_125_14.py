#!/usr/bin/python3

import sys
import time
import struct
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('I2C:DEV80 "/dev/i2c-0"')
print("Init I2C")

rp_s.tx_txt('I2C:DEV?')
print(f"Device address: {rp_s.rx_txt()}")

rp_s.tx_txt('I2C:FMODE ON')
print("Set force mode")
rp_s.check_error()
# Eeprom 24c64 supports reading only 32 bytes of data at a time and only works through IOCTL

# set read address = 0
rp_s.tx_txt('I2C:IO:W:B2 0,0')
print("Write address for read")

rp_s.tx_txt('I2C:IO:R:B32?')
b1 = rp_s.rx_txt().strip('{').strip('}')

rp_s.tx_txt('I2C:IO:R:B16?')
b2 = rp_s.rx_txt().strip('{').strip('}')
rp_s.check_error()

buff = (b1 + "," + b2).split(",")
byte_array = bytearray(b'')
for s in buff:
    byte_array.append(int(s))

calib = [struct.unpack('i', byte_array[i:i+4])[0] for i in range(0, len(byte_array), 4)]
calibId, wp, res1, res2 = (calib[0] & 0xFFFFFFFF).to_bytes(4, 'little')

if (calibId == 1):
    print("ADC Ch1 High", calib[2])
    print("ADC Ch2 High", calib[3])
    print("ADC Ch1 Low", calib[4])
    print("ADC Ch2 Low", calib[5])
    print("ADC Ch1 Low offset", calib[6])
    print("ADC Ch2 Low offset", calib[7])
    print("DAC Ch1", calib[8])
    print("DAC Ch2", calib[9])
    print("DAC Ch1 offset", calib[10])
    print("DAC Ch2 offset", calib[11])
else:
    print("Unknown format of calibration. recived data:",byte_array)