
#!/usr/bin/python3

import rp_la
import sys
import numpy as np

obj = rp_la.CLAController()

obj.loadFromFile(sys.argv[1] + '/' + sys.argv[2] + ".bin" ,True ,0)

obj.addDecoder("CAN",rp_la.LA_DECODER_CAN)
f = open(sys.argv[1] +"/settings.json", "r")
obj.setDecoderSettings("CAN",f.read())
res = obj.decode("CAN")
idx = 0
crc = 15
for x in res:
    if x['control'] == 3:
        print('can-1: Identifier: {:d} (0x{:x})'.format(x['data'],x['data']))
        idx = 0
    if x['control'] == 4:
        print('can-1: Extended Identifier: {:d} (0x{:02x})'.format(x['data'],x['data']))
    if x['control'] == 5:
        print('can-1: Full Identifier: {:d} (0x{:02x})'.format(x['data'],x['data']))
    if x['control'] == 0:
        print('can-1: Data byte {:d}: 0x{:02x}'.format(idx,x['data']))
        idx = idx + 1
    if x['control'] == 23:
        print('can-1: CRC-15 sequence: 0x{:04x}'.format(x['data']))
    if x['control'] == 24:
        crc = 17
    if x['control'] == 25:
        crc = 21
    if x['control'] == 28:
        print('can-1: CRC-{:d} sequence: 0x{:04x}'.format(crc,x['data']))
del obj