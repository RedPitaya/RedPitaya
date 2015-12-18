#!/usr/bin/python

import redpitaya_scpi as scpi
import time
import sys

__author__ = "Luka Golinar, Iztok Jeras"
__copyright__ = "Copyright 2015, Red Pitaya"

rp_scpi = scpi.scpi(sys.argv[1])

buff = []
buff_ctrl = []
channel = 1

#Do not change this values!
freq = 7629.39453125
ampl = 1
wave_form = 'SINE'

#Enable Red Pitaya digital loop
rp_scpi.tx_txt('OSC:DIGLOOP')

rp_scpi.tx_txt('ACQ:START')
rp_scpi.tx_txt('ACQ:TRIG CH1_PE')

#Set generator options
rp_scpi.tx_txt('SOUR' + str(channel) + ':FREQ:FIX ' + str(freq))
rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT ' + str(ampl))
rp_scpi.tx_txt('SOUR' + str(channel) + ':FUNC ' + str(wave_form))
rp_scpi.tx_txt('OUTPUT' + str(channel) + ':STATE ON')

rp_scpi.tx_txt('ACQ:TRIG:STAT?')
rp_scpi.rx_txt()
rp_scpi.tx_txt('ACQ:SOUR' + str(channel) + ':DATA?')

buff_string = rp_scpi.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = map(float, buff_string[:1024])

import matplotlib.pyplot as plt
plt.plot(buff)
plt.ylabel('Voltage')
plt.show()
