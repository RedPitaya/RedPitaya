#!/usr/bin/env python

__author__ = 'infused'

import sys
from time import sleep
from redpitaya_scpi import scpi

rp_s = scpi (sys.argv[1])

wave_form = 'sine'
freq = 10000
ampl = 0.9

#rp_s.tx_txt('OSC:RUN:DIGLOOP')

rp_s.tx_txt('SOUR1:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR1:FREQ:FIX ' + str(freq))
rp_s.tx_txt('SOUR1:VOLT ' + str(ampl))

rp_s.tx_txt('ACQ:RST')

rp_s.tx_txt('ACQ:SOUR1:GAIN LV')
rp_s.tx_txt('ACQ:DEC 1'       )
rp_s.tx_txt('ACQ:TRIG:LEV 0 mV')
rp_s.tx_txt('ACQ:START'       )
rp_s.tx_txt('ACQ:TRIG CH1_PE' )
while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT?')
    if rp_s.rx_txt() == 'TD':
        break
rp_s.tx_txt('ACQ:SOUR1:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{} ').split(',')
buff = map(float, buff_string)

# check trigger settings
rp_s.tx_txt('ACQ:TRIG:DLY?')
print('ACQ:TRIG:DLY? = ' + rp_s.rx_txt())

rp_s.tx_txt('ACQ:SOUR1:GAIN?')
print('ACQ:SOUR1:GAIN = ' + rp_s.rx_txt())

import matplotlib.pyplot as plt
plt.plot(buff)
plt.ylabel('Voltage')
plt.show()
