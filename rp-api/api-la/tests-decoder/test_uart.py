
#!/usr/bin/python3

import rp_la
import sys
import numpy as np

obj = rp_la.CLAController()

obj.loadFromFile(sys.argv[1] + '/' + sys.argv[2] + ".bin" ,True ,0)

obj.addDecoder("UART",rp_la.LA_DECODER_UART)
f = open(sys.argv[1] +"/settings.json", "r")
obj.setDecoderSettings("UART",f.read())
bits = obj.getDecoderSettingsUInt("UART","num_data_bits")[1]
res = obj.decode("UART")
for x in res:
    if x['control'] == 4:
        print('uart-1: Start bit')
    if x['control'] == 5:
        print('uart-1: Stop bit')
    if x['control'] == 6:
        print('uart-1: Parity bit')
    if x['control'] == 2:
        print('uart-1: Frame error')
        print('uart-1: Stop bit')
    if x['control'] == 3:
        if (bits < 9):
            print('uart-1: {:02X}'.format(x['data']))
        else:
            print('uart-1: {:03X}'.format(x['data']))
del obj