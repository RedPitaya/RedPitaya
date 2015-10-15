
import redpitaya_scpi as scpi
import sys
import array
import matplotlib.pyplot as plt

rp_s = scpi.scpi('192.168.178.108')

wave_form = 'sine'
freq = 100000
ampl = 1

rp_s.tx_txt('*RST')

rp_s.tx_txt('SOUR1:FUNC ' + str(wave_form).upper())
rp_s.tx_txt('SOUR1:FREQ:FIX ' + str(freq))
rp_s.tx_txt('SOUR1:VOLT ' + str(ampl))

#Enable output
rp_s.tx_txt('OUTPUT1:STATE ON')

rp_s.tx_txt('ACQ:START')
rp_s.tx_txt('ACQ:TRIG NOW')
rp_s.tx_txt('ACQ:TRIG:STAT?')
rp_s.rx_txt()

################################################################################
# float ascii

rp_s.tx_txt('ACQ:DATA:UNITS VOLTS')
rp_s.tx_txt('ACQ:DATA:FORMAT ASCII')

rp_s.tx_txt('ACQ:SOUR1:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = map(float, buff_string)

print "FLOAT ASCII"
print buff_string
print(buff[:10])
plt.plot(buff)
plt.ylabel('Voltage')
plt.show()

################################################################################
# int16_t ascii

rp_s.tx_txt('ACQ:DATA:UNITS RAW')
rp_s.tx_txt('ACQ:DATA:FORMAT ASCII')

rp_s.tx_txt('ACQ:SOUR1:DATA?')
buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = map(int, buff_string)

print "INT16 ASCII"
print buff_string
print(buff[:10])
plt.plot(buff)
plt.ylabel('Voltage')
plt.show()

################################################################################
# float bin

rp_s.tx_txt('ACQ:DATA:UNITS VOLTS')
rp_s.tx_txt('ACQ:DATA:FORMAT BIN')

rp_s.tx_txt('ACQ:SOUR1:DATA?')
str = rp_s.rx_arb()
print [hex(ord(str[i])) for i in range(10)]
buff = array.array('f', str)

print "FLOAT BIN"
print(len(str))
print(buff[:10])
plt.plot(buff[:])
plt.ylabel('Voltage')
plt.show()

################################################################################
# int16_t bin

rp_s.tx_txt('ACQ:DATA:UNITS RAW')
rp_s.tx_txt('ACQ:DATA:FORMAT BIN')

rp_s.tx_txt('ACQ:SOUR1:DATA?')
str = rp_s.rx_arb()
print [hex(ord(str[i])) for i in range(10)]
buff = array.array('h', str)

print "INT16 BIN"
print(len(str))
print(buff[:10])
plt.plot(buff[:])
plt.ylabel('Voltage')
plt.show()
