#!/usr/bin/python

import sys
import redpitaya_scpi as scpi
import matplotlib.pyplot as plot
import struct

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('ACQ:DATA:FORMAT BIN')
rp_s.tx_txt('ACQ:DATA:UNITS VOLTS')
rp_s.tx_txt('ACQ:DEC 8')

rp_s.tx_txt('ACQ:START')
rp_s.tx_txt('ACQ:TRIG NOW')

while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT?')
    if rp_s.rx_txt() == 'TD':
        break

rp_s.tx_txt('ACQ:SOUR1:DATA?')
buff_byte = rp_s.rx_arb()
buff = [struct.unpack('!f',bytearray(buff_byte[i:i+4]))[0] for i in range(0, len(buff_byte), 4)]

plot.plot(buff)
plot.ylabel('Voltage')
plot.show()
