#!/usr/bin/python3

import sys
import redpitaya_scpi as scpi
import matplotlib.pyplot as plot

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('ACQ:RST')
rp_s.tx_txt('ACQ:DATA:FORMAT ASCII')
rp_s.tx_txt('ACQ:DATA:UNITS VOLTS')

rp_s.tx_txt('ACQ:DEC 1')
rp_s.tx_txt('ACQ:TRIG:LEV 0');
rp_s.tx_txt('ACQ:TRIG:DLY 0');

rp_s.tx_txt('ACQ:START')
rp_s.tx_txt('ACQ:TRIG CH1_PE')

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

rp_s.tx_txt('ACQ:SOUR2:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff2 = list(map(float, buff_string))

rp_s.tx_txt('ACQ:SOUR3:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff3 = list(map(float, buff_string))

rp_s.tx_txt('ACQ:SOUR4:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff4 = list(map(float, buff_string))

plot.plot(buff, 'r')
plot.plot(buff2, 'g')
plot.plot(buff3, 'b')
plot.plot(buff4, 'm')
plot.ylabel('Voltage')
plot.show()
