#!/usr/bin/python

import sys
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('UART:INIT')
print("Init UART")


rp_s.tx_txt('UART:BITS CS7')
print("Set bit size CS7")

rp_s.tx_txt('UART:BITS?')
print("Check bit size",rp_s.rx_txt())

rp_s.tx_txt('UART:SPEED 57600')
print("Set speed 57600")

rp_s.tx_txt('UART:SPEED?')
print("Check speed",rp_s.rx_txt())

rp_s.tx_txt('UART:STOPB STOP2')
print("Set stop bit STOP2")

rp_s.tx_txt('UART:STOPB?')
print("Check stop bit",rp_s.rx_txt())

rp_s.tx_txt('UART:PARITY ODD')
print("Set parity mode: ODD")

rp_s.tx_txt('UART:PARITY?')
print("Check parity mode",rp_s.rx_txt())

rp_s.tx_txt('UART:TIMEOUT 10')
print("Set timeout: 10 decams")

rp_s.tx_txt('UART:TIMEOUT?')
print("Check timeout",rp_s.rx_txt())


rp_s.tx_txt('UART:SETUP')
print("Setup settings")

rp_s.tx_txt('UART:WRITE7 #H11,#H22,#H33,33,33,#Q11,#B11001100')
print("Write 7 bytes to uart: #H11,#H22,#H33,33,33,#Q11,#B11001100'")

rp_s.tx_txt('UART:READ3')
print("Read: ",rp_s.rx_txt())

rp_s.tx_txt('UART:READ4')
print("Read: ",rp_s.rx_txt())

rp_s.tx_txt('UART:RELEASE')
print("Release UART")

