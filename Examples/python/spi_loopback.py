#!/usr/bin/python3

import sys
import time
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('SPI:INIT:DEV "/dev/spidev1.0"')
print("Init SPI")

rp_s.tx_txt('SPI:SET:DEF')
print("Set default settings")

rp_s.tx_txt('SPI:SET:GET')
print("Get settings")

rp_s.tx_txt('SPI:SET:MODE LIST')
print("Set mode")

rp_s.tx_txt('SPI:SET:MODE?')
print("Get mode:",rp_s.rx_txt())


rp_s.tx_txt('SPI:SET:SPEED 5000000')
print("Set speed")

rp_s.tx_txt('SPI:SET:SPEED?')
print("Get speed:",rp_s.rx_txt())

rp_s.tx_txt('SPI:SET:WORD 8')
print("Set word length")

rp_s.tx_txt('SPI:SET:WORD?')
print("Get word length:",rp_s.rx_txt())

rp_s.tx_txt('SPI:SET:SET')
print("Set settings")

rp_s.tx_txt('SPI:MSG:CREATE 2')
print("Create message")

rp_s.tx_txt('SPI:MSG:SIZE?')
print("Message size:",rp_s.rx_txt())

rp_s.tx_txt('SPI:MSG0:TX4:RX 13,14,15,16')
print("Set message")

rp_s.tx_txt('SPI:MSG1:RX7:CS')
print("Set message 2")

rp_s.tx_txt('SPI:PASS')
print("Pass message")

rp_s.tx_txt('SPI:MSG0:TX?')
print("Tx buffer:",rp_s.rx_txt())

rp_s.tx_txt('SPI:MSG0:RX?')
print("Received data:",rp_s.rx_txt())

rp_s.tx_txt('SPI:MSG1:RX?')
print("Received data 2:",rp_s.rx_txt())

rp_s.tx_txt('SPI:MSG1:CS?')
print("CS state for message 2:",rp_s.rx_txt())

rp_s.tx_txt('SPI:MSG:DEL')
print("Delete message")

rp_s.tx_txt('SPI:RELEASE')
print("Release SPI")

