#!/usr/bin/python3

import rp_la
import sys
import numpy as np

obj = rp_la.CLAController()

obj.loadFromFile(sys.argv[1] + '/' + sys.argv[2] + ".bin" ,True ,0)

obj.addDecoder("SPI",rp_la.LA_DECODER_SPI)
f = open(sys.argv[1] +"/settings.json", "r")
obj.setDecoderSettings("SPI",f.read())
res = obj.decode("SPI")
for x in res:
    if x['control'] == 0:
        print('spi-1: {:02X}'.format(x['data']))
del obj