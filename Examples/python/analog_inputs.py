#!/usr/bin/python

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

for i in range(4):
    rp_s.tx_txt('ANALOG:PIN? AIN' + str(i))
    value = float(rp_s.rx_txt())
    print ("Measured voltage on AI["+str(i)+"] = "+str(value)+"V")
