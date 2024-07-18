#!/usr/bin/python3

import sys
import redpitaya_scpi as scpi
import matplotlib.pyplot as plot

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('ACQ:SPLIT:TRig ON')
rp_s.tx_txt('ACQ:RST:CH1')
rp_s.tx_txt('ACQ:RST:CH2')
rp_s.tx_txt('ACQ:RST:CH3')
rp_s.tx_txt('ACQ:RST:CH4')
rp_s.tx_txt('ACQ:DATA:FORMAT ASCII')
rp_s.tx_txt('ACQ:DATA:UNITS VOLTS')
rp_s.tx_txt('ACQ:DEC:CH1 64')
rp_s.tx_txt('ACQ:DEC:CH2 64')
rp_s.tx_txt('ACQ:DEC:CH3 64')
rp_s.tx_txt('ACQ:DEC:CH4 64')

rp_s.tx_txt('ACQ:START:CH1')
rp_s.tx_txt('ACQ:START:CH2')
rp_s.tx_txt('ACQ:START:CH3')
rp_s.tx_txt('ACQ:START:CH4')
rp_s.tx_txt('ACQ:TRIG:CH1 CH1_PE')
rp_s.tx_txt('ACQ:TRIG:CH2 CH2_PE')
rp_s.tx_txt('ACQ:TRIG:CH3 CH3_NE')
rp_s.tx_txt('ACQ:TRIG:CH4 CH4_NE')

while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT:CH1?')
    if rp_s.rx_txt() == 'TD':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:FILL:CH1?')
    if rp_s.rx_txt() == '1':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT:CH2?')
    if rp_s.rx_txt() == 'TD':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:FILL:CH2?')
    if rp_s.rx_txt() == '1':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT:CH3?')
    if rp_s.rx_txt() == 'TD':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:FILL:CH3?')
    if rp_s.rx_txt() == '1':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT:CH4?')
    if rp_s.rx_txt() == 'TD':
        break

while 1:
    rp_s.tx_txt('ACQ:TRIG:FILL:CH4?')
    if rp_s.rx_txt() == '1':
        break

rp_s.tx_txt('ACQ:STOP:CH1')
rp_s.tx_txt('ACQ:STOP:CH2')
rp_s.tx_txt('ACQ:STOP:CH3')
rp_s.tx_txt('ACQ:STOP:CH4')

rp_s.tx_txt('ACQ:SOUR1:DATA:TR? 6000,POST_TRIG')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = list(map(float, buff_string))

rp_s.tx_txt('ACQ:SOUR2:DATA:TR? 6000,POST_TRIG')
buff_string2 = rp_s.rx_txt()
buff_string2 = buff_string2.strip('{}\n\r').replace("  ", "").split(',')
buff2 = list(map(float, buff_string2))

rp_s.tx_txt('ACQ:SOUR3:DATA:TR? 6000,POST_TRIG')
buff_string3 = rp_s.rx_txt()
buff_string3 = buff_string3.strip('{}\n\r').replace("  ", "").split(',')
buff3 = list(map(float, buff_string3))

rp_s.tx_txt('ACQ:SOUR4:DATA:TR? 6000,POST_TRIG')
buff_string4 = rp_s.rx_txt()
buff_string4 = buff_string4.strip('{}\n\r').replace("  ", "").split(',')
buff4 = list(map(float, buff_string4))

fig, axs = plot.subplots(4)
fig.suptitle('ADC data')
axs[0].plot(buff)
axs[1].plot(buff2)
axs[2].plot(buff3)
axs[3].plot(buff4)
plot.show()
