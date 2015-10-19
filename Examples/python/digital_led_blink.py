#!/usr/bin/python

import redpitaya_scpi as scpi
import time
import sys

__author__ = "Luka Golinar, Iztok Jeras"
__copyright__ = "Copyright 2015, Red Pitaya"

rp_s = scpi.scpi(sys.argv[1])

time_out = 1#seconds
diode = 5

while 1:
    time.sleep(time_out)
    rp_s.tx_txt(rp_s.choose_state(diode, 1))
    time.sleep(time_out)
    rp_s.tx_txt(rp_s.choose_state(diode, 0))
