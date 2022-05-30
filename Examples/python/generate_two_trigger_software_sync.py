#!/usr/bin/python

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi("192.168.1.17")

wave_form = 'sine'
freq = 2000
ampl = 1

rp_s.tx_txt('GEN:RST')

rp_s.tx_txt('SOUR1:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR1:FREQ:FIX ' + str(freq))
rp_s.tx_txt('SOUR1:VOLT ' + str(ampl))
rp_s.tx_txt('SOUR2:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR2:FREQ:FIX ' + str(freq))
rp_s.tx_txt('SOUR2:VOLT ' + str(ampl))

rp_s.tx_txt('OUTPUT:STATE ON')
rp_s.tx_txt('SOUR:TRIG:INT')
