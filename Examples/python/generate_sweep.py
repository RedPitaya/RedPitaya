#!/usr/bin/python3

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])


wave_form = 'sine'
ampl = 1

rp_s.tx_txt('GEN:RST')
rp_s.tx_txt('SOUR1:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR1:VOLT ' + str(ampl))

#Enable output
rp_s.tx_txt('OUTPUT1:STATE ON')
rp_s.tx_txt('SOUR1:TRIG:INT')

rp_s.tx_txt('SOUR:SWEEP:RESET')


rp_s.tx_txt('SOUR1:SWEEP:FREQ:START 1000')
rp_s.tx_txt('SOUR1:SWEEP:FREQ:STOP 1000000')
rp_s.tx_txt('SOUR1:SWEEP:TIME 10000000')

rp_s.tx_txt('SOUR1:SWEEP:MODE LOG')
rp_s.tx_txt('SOUR1:SWEEP:DIR UP_DOWN')
rp_s.tx_txt('SOUR1:SWEEP:STATE ON')
