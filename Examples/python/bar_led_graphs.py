#!/usr/bin/python

import redpitaya_scpi as scpi
import time
import sys

__author__ = "Luka Golinar, Iztok Jeras"
__copyright__ = "Copyright 2015, Red Pitaya"

rp_s = scpi.scpi(sys.argv[1])

test_parameter = 67

for i in range(1, 7):
    if(test_parameter >= ((100/7) * i)):
        rp_s.tx_txt(rp_s.choose_state(str(i), 1))
    else:
        rp_s.tx_txt(rp_s.choose_state(str(i), 0))
