#!/usr/bin/python

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

# set all DIO*_N pins to inputs
for i in range(8):
    rp_s.tx_txt('DIG:PIN:DIR IN,DIO'+str(i)+'_N')

# copy DIOi_N pin state to LEDi state fir each i [0:7]
while 1:
    for i in range(8):
        rp_s.tx_txt('DIG:PIN? DIO'+str(i)+'_N')
        state = rp_s.rx_txt()
        rp_s.tx_txt('DIG:PIN LED'+str(i)+','+str(state))
