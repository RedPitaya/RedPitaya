
#!/usr/bin/python3

import rp_la
import sys
import numpy as np

obj = rp_la.CLAController()

obj.loadFromFile(sys.argv[1] + '/' + sys.argv[2] + ".bin" ,True ,0)

obj.addDecoder("I2C",rp_la.LA_DECODER_I2C)
f = open(sys.argv[1] +"/settings.json", "r")
obj.setDecoderSettings("I2C",f.read())
res = obj.decode("I2C")
for x in res:
    if x['control'] == 0:
        print('i2c-1: Start')
    if x['control'] == 1:
        print('i2c-1: Start repeat')
    # if x['control'] == 2:
    #     print('i2c-1: Stop')
    if x['control'] == 3:
        print('i2c-1: ACK')
    if x['control'] == 4:
        print('i2c-1: NACK')
    if x['control'] == 5:
        print('i2c-1: Address read: {:02X}'.format(x['data']))
    if x['control'] == 6:
        print('i2c-1: Address write: {:02X}'.format(x['data']))
    if x['control'] == 7:
        print('i2c-1: Data read: {:02X}'.format(x['data']))
    if x['control'] == 8:
        print('i2c-1: Data write: {:02X}'.format(x['data']))
del obj