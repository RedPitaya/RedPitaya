#!/usr/bin/python3

import sys
import time
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('RP:LOGmode CONSOLE')

rp_s.tx_txt('DAISY:ENable ON')
print("DAISY:ENable? ON")

rp_s.tx_txt('DAISY:ENable?')
print("DAISY:ENable",rp_s.rx_txt())


rp_s.tx_txt('DAISY:TRIG_O:ENable ON')
print("DAISY:TRIG_O:ENable ON")

rp_s.tx_txt('DAISY:TRIG_O:ENable?')
print("DAISY:TRIG_O:ENable?",rp_s.rx_txt())

rp_s.tx_txt('DAISY:TRIG_O:SOUR DAC')
print("DAISY:TRIG_O:SOUR DAC")

rp_s.tx_txt('DAISY:TRIG_O:SOUR?')
print("DAISY:TRIG_O:SOUR?",rp_s.rx_txt())


rp_s.tx_txt('DAISY:TRIG_O:SOUR ADC')
print("DAISY:TRIG_O:SOUR ADC")

rp_s.tx_txt('DAISY:TRIG_O:SOUR?')
print("DAISY:TRIG_O:SOUR?",rp_s.rx_txt())


rp_s.tx_txt('ACQ:TRIG:EXT:DEBouncerUs?')
print("ACQ:TRIG:EXT:DEBouncerUs?",rp_s.rx_txt())

rp_s.tx_txt('ACQ:TRIG:EXT:DEBouncerUs 2.5')
print("ACQ:TRIG:EXT:DEBouncerUs 2.5")

rp_s.tx_txt('ACQ:TRIG:EXT:DEBouncerUs?')
print("ACQ:TRIG:EXT:DEBouncerUs?",rp_s.rx_txt())

rp_s.tx_txt('SOUR:TRIG:EXT:DEBouncerUs?')
print("SOUR:TRIG:EXT:DEBouncerUs?",rp_s.rx_txt())

rp_s.tx_txt('SOUR:TRIG:EXT:DEBouncerUs 2.5')
print("SOUR:TRIG:EXT:DEBouncerUs 2.5")

rp_s.tx_txt('SOUR:TRIG:EXT:DEBouncerUs?')
print("SOUR:TRIG:EXT:DEBouncerUs?",rp_s.rx_txt())

rp_s.tx_txt('DAISY:ENable OFF')
print("DAISY:ENable OFF")

rp_s.tx_txt('DAISY:ENable?')
print("DAISY:ENable?",rp_s.rx_txt())
