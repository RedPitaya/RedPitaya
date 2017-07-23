#!/usr/bin/python

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

if (len(sys.argv) > 2):
  percent = int(sys.argv[2])
else:
  percent = 50

print ("Bar showing "+str(percent)+"%")

for i in range(8):
    if (percent > (i * (100.0/8))):
        rp_s.tx_txt('DIG:PIN LED' + str(i) + ',' + str(1))
    else:
        rp_s.tx_txt('DIG:PIN LED' + str(i) + ',' + str(0))
