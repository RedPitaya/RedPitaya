
__author__ = 'infused'

from time import sleep
from redpitaya_scpi import scpi

rp_s = scpi ('192.168.178.36')

wave_form = 'sine'
freq = 5000
ampl = 1

#Generate signal
rp_s.send_txt('SOUR1:FUNC '      + str(wave_form).upper())
rp_s.send_txt('SOUR1:FREQ:FIX '  + str(freq)             )
rp_s.send_txt('SOUR1:VOLT '      + str(ampl)             )
rp_s.send_txt('OUTPUT1:STATE ON'                         )

buff = []
buff_string = ''
rp_s.send_txt('ACQ:DEC 8'       )
rp_s.send_txt('ACQ:TRIG:LEV 100')
rp_s.send_txt('ACQ:START'       )
rp_s.send_txt('ACQ:TRIG CH1_PE' )
while 1:
    rp_s.send_txt('ACQ:TRIG:STAT?')
    if rp_s.recv_txt(2) == 'TD':
        break
rp_s.send_txt('ACQ:SOUR1:DATA?')
buff_string = rp_s.recv_txt()
buff_string = buff_string.strip('{} ').split(',')
buff = map(float, buff_string)

import matplotlib.pyplot as plt
plt.plot(buff)
plt.ylabel('Voltage')
plt.show()
