#!/usr/bin/python

import redpitaya_scpi as scpi
import time
import sys

__author__ = "Luka Golinar, Iztok Jeras"
__copyright__ = "Copyright 2015, Red Pitaya"

rp_s = scpi.scpi(sys.argv[1])

wave_form = 'sine'
freq = 1000
ampl = 1

for i in range(0, 1000):
    rp_s.tx_txt('SOUR1:FUNC ' + str(wave_form).upper())
    rp_s.tx_txt('SOUR1:FREQ:FIX ' + str(freq))
    rp_s.tx_txt('SOUR1:VOLT ' + str(ampl))
    rp_s.tx_txt('SOUR1:BURS:NCYC 1')
    rp_s.tx_txt('SOUR1:TRIG:IMM')

    #Enable output
    rp_s.tx_txt('OUTPUT1:STATE ON')
    rp_s.tx_txt('SOUR1:TRIG:IMM')

    #Sleep for N time before generating another pulse
    time.sleep(10)