#!/usr/bin/python3

import sys
import time
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

wave_form = 'sine'
freq = 4
ampl = 1

rp_s.tx_txt('GEN:RST')

rp_s.tx_txt('SOUR1:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR1:FREQ:FIX ' + str(freq))
rp_s.tx_txt('SOUR1:VOLT ' + str(ampl))

rp_s.tx_txt('SOUR2:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR2:FREQ:FIX ' + str(freq))
rp_s.tx_txt('SOUR2:VOLT ' + str(ampl))

rp_s.tx_txt('SOUR1:BURS:STAT BURST')
rp_s.tx_txt('SOUR1:BURS:NCYC 2')
rp_s.tx_txt('SOUR1:BURS:NOR 1')
rp_s.tx_txt('SOUR1:BURS:INT:PER 5000')

rp_s.tx_txt('SOUR2:BURS:STAT BURST')
rp_s.tx_txt('SOUR2:BURS:NCYC 2')
rp_s.tx_txt('SOUR2:BURS:NOR 1')
rp_s.tx_txt('SOUR2:BURS:INT:PER 5000')

rp_s.tx_txt('OUTPUT:STATE ON')
rp_s.tx_txt('SOUR:TRIG:INT')
time.sleep(2)
rp_s.tx_txt('SOUR1:TRIG:INT')
time.sleep(2)
rp_s.tx_txt('SOUR2:TRIG:INT')
time.sleep(1)
rp_s.tx_txt('SOUR:TRIG:INT')
