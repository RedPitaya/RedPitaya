#!/usr/bin/python

import redpitaya_scpi as scpi
import time
import sys

rp_s = scpi.scpi(sys.argv[1])

wave_form = 'sine'
freq = 10000
ampl = 1

rp_s.tx_txt('GEN:RST')
rp_s.tx_txt('SOUR2:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR2:FREQ:FIX ' + str(freq))
rp_s.tx_txt('SOUR2:VOLT ' + str(ampl))
rp_s.tx_txt('SOUR2:BURS:NCYC 2')
rp_s.tx_txt('OUTPUT2:STATE ON')
rp_s.tx_txt('SOUR2:BURS:STAT ON')
rp_s.tx_txt('SOUR2:TRIG:SOUR EXT_PE')