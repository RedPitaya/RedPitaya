#!/usr/bin/python

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

value = [1,1,1,1]
for i in range(4):
    if len(sys.argv) > (i+2):
        value[i] = sys.argv[i+2]
    print ("Voltage setting for AO["+str(i)+"] = "+str(value[i])+"V")

for i in range(4):
    rp_s.tx_txt('ANALOG:PIN AOUT' + str(i) + ',' + str(value[i]))
