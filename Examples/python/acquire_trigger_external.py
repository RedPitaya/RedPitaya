#!/usr/bin/python3

import sys
import time
import redpitaya_scpi as scpi
import matplotlib.pyplot as plot

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('ACQ:RST')
rp_s.tx_txt('ACQ:DEC 1')
rp_s.tx_txt('ACQ:TRIG:LEV 0')
rp_s.tx_txt('ACQ:TRIG:DLY 0')

rp_s.tx_txt('ACQ:START')

time.sleep(1)

rp_s.tx_txt('ACQ:TRIG EXT_PE')

while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT?')
    if rp_s.rx_txt() == 'TD':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:FILL?')
    if rp_s.rx_txt() == '1':
        break

rp_s.tx_txt('ACQ:SOUR1:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = list(map(float, buff_string))

plot.plot(buff)
plot.ylabel('Voltage')
plot.show()
