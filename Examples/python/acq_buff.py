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

buff = []
buff_string = ''
rp_s.tx_txt('ACQ:START')
rp_s.tx_txt('ACQ:TRIG NOW')
rp_s.tx_txt('ACQ:TRIG:STAT?')
rp_s.rx_txt()
rp_s.tx_txt('ACQ:SOUR1:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = map(float, buff_string)