import redpitaya_scpi as scpi
import sys
import math


rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('ACQ:BUF:SIZE?')
BUFF_SIZE = int(rp_s.rx_txt())

y = ''
x = ''
t = []

for i in range(0, BUFF_SIZE):
	t.append((2 * math.pi) / BUFF_SIZE * i)

for i in range(0, BUFF_SIZE-1):
	if(i != BUFF_SIZE-2): 
		b = math.sin(t[i]) + (1.0/3.0) + math.sin(t[i] * 3)
		if(b <= -1 or b >= 1):
			x += str(-1.0) + ', '
			y += str(-1.0) + ', '
		else: 
			x += str(math.sin(t[i]) + (1.0/3.0) + math.sin(t[i] * 3)) + ', '
			y += str((1.0 / 2.0) * math.sin(t[i]) + (1.0/4.0) * math.sin(t[i] * 4)) + ', '
			
	else:
		c = math.sin(t[i]) + (1.0/3.0) + math.sin(t[i] * 3)
		if(c <= -1 or c >= 1):
			x += str(-1.0)
			y += str(-1.0) 
		else:
			x += str(math.sin(t[i]) + (1.0/3.0) + math.sin(t[i] * 3))
			y += str((1.0 / 2.0) * math.sin(t[i]) + (1.0/4.0) * math.sin(t[i] * 4))		

rp_s.tx_txt('SOUR1:FUNC ARBITRARY')
rp_s.tx_txt('SOUR2:FUNC ARBITRARY')

rp_s.tx_txt('SOUR1:TRAC:DATA:DATA ' + x)
rp_s.tx_txt('SOUR2:TRAC:DATA:DATA ' + y)

rp_s.tx_txt('SOUR1:VOLT 1')
rp_s.tx_txt('SOUR2:VOLT 1')

rp_s.tx_txt('SOUR1:FREQ:FIX 4000')
rp_s.tx_txt('SOUR2:FREQ:FIX 4000')

rp_s.tx_txt('OUTPUT1:STATE ON')
rp_s.tx_txt('OUTPUT2:STATE ON')

#print x

rp_s.tx_txt('SOUR1:TRAC:DATA:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = map(float, buff_string)

import matplotlib.pyplot as plt
plt.plot(buff)
plt.ylabel('Voltage')
plt.show()
