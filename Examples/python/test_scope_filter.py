
__author__ = 'infused'

from redpitaya_scpi import scpi

rp_s = scpi ('192.168.178.36')

wave_form = 'sine'
freq = 1000
ampl = 1

buff = []
buff_string = ''
rp_s.send('ACQ:DEC 8'        + rp_s.delimiter)
rp_s.send('ACQ:TRIG:LEV 100' + rp_s.delimiter)
rp_s.send('ACQ:START'        + rp_s.delimiter)
rp_s.send('ACQ:TRIG CH1_PE'  + rp_s.delimiter)

while 1:
    rp_s.send('ACQ:TRIG:STAT?' + rp_s.delimiter)
    if rp_s.recv(2) == 'TD':
        break

rp_s.send('ACQ:SOUR1:DATA?' + rp_s.delimiter)
buff_string = rp_s.recv()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = map(float, buff_string)

