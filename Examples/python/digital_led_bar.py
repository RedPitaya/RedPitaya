#!/usr/bin/python

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])
bar = int(sys.argv[2])

print bar
for i in range(8):
    if (bar > (i * (100.0/8))):
        rp_s.tx_txt('DIG:PIN LED' + str(i) + ',' + str(1))
    else:
        rp_s.tx_txt('DIG:PIN LED' + str(i) + ',' + str(0))
