#!/usr/bin/python

import redpitaya_scpi as scpi
import time
import sys

__author__ = "Luka Golinar, Iztok Jeras"
__copyright__ = "Copyright 2015, Red Pitaya"

rp_s = SCPI('192.168.178.36', 0.5)

rp_s.tx_txt('DIG:PIN:DIR OUT,DIO5_N')


while 1:
    rp_s.tx_txt('DIG:PIN? DIO5_N')
    if rp_s.rx_txt()[0] == '1':
        rp_s.tx_txt('DIG:PIN LED5,0')
    else:
        rp_s.tx_txt('DIG:PIN LED5,1')