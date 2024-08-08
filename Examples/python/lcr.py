#!/usr/bin/python3

import sys
import time
import json
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

# Need set before run
print("LCR:SHUNT:MODE CUSTOM")
rp_s.tx_txt('LCR:SHUNT:MODE CUSTOM')
rp_s.check_error()

rp_s.tx_txt('LCR:EXT:MODULE?')
print('LCR:EXT:MODULE?', rp_s.rx_txt())
rp_s.check_error()

rp_s.tx_txt('LCR:START')
print("LCR:START")
rp_s.check_error()

print("LCR:SHUNT:AUTO OFF")
rp_s.tx_txt('LCR:SHUNT:AUTO OFF')
rp_s.check_error()

print("LCR:SHUNT:CUSTOM 10")
rp_s.tx_txt("LCR:SHUNT:CUSTOM 10")
rp_s.check_error()

rp_s.tx_txt('LCR:SHUNT:CUSTOM?')
print('LCR:SHUNT:CUSTOM?', rp_s.rx_txt())
rp_s.check_error()

rp_s.tx_txt('LCR:FREQ 1000')
print("LCR:FREQ 1000")
rp_s.check_error()

rp_s.tx_txt('LCR:FREQ?')
print('LCR:FREQ?', rp_s.rx_txt())
rp_s.check_error()

rp_s.tx_txt('LCR:VOLT 0.5')
print("LCR:VOLT 0.5")
rp_s.check_error()

rp_s.tx_txt('LCR:VOLT?')
print('LCR:VOLT?', rp_s.rx_txt())
rp_s.check_error()

rp_s.tx_txt('LCR:VOLT:OFFS 0')
print("LCR:VOLT:OFFS 0")
rp_s.check_error()

rp_s.tx_txt('LCR:VOLT:OFFS?')
print('LCR:VOLT:OFFS?', rp_s.rx_txt())
rp_s.check_error()

rp_s.tx_txt('LCR:CIRCUIT SERIES')
print("LCR:CIRCUIT SERIES")
rp_s.check_error()

rp_s.tx_txt('LCR:CIRCUIT?')
print('LCR:CIRCUIT?', rp_s.rx_txt())
rp_s.check_error()

# Start the generator after changing the settings.
rp_s.tx_txt('LCR:START:GEN')
print("LCR:START:GEN")
rp_s.check_error()

time.sleep(1)
rp_s.tx_txt('LCR:MEASURE?')
data = json.loads(rp_s.rx_txt())
print('LCR:MEASURE?', data)
print('Freq', data['freq'])
print('R_s', data['R_s'])
rp_s.check_error()

rp_s.tx_txt('LCR:STOP')
print("LCR:STOP")
rp_s.check_error()
