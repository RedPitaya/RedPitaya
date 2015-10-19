#!/usr/bin/python

import sys
import time
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

period = 1 # seconds
led = 5

while 1:
    time.sleep(period/2.0)
    rp_s.tx_txt('DIG:PIN LED' + str(led) + ',' + str(1))
    time.sleep(period/2.0)
    rp_s.tx_txt('DIG:PIN LED' + str(led) + ',' + str(0))
