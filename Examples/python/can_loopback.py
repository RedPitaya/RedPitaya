#!/usr/bin/python3

import sys
import time
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('CAN:FPGA ON')
print("CAN:FPGA ON")
rp_s.check_error()

rp_s.tx_txt('CAN0:STOP')
print("CAN0:START")
rp_s.check_error()

rp_s.tx_txt('CAN0:BITRate 200000')
print("CAN0:BitRate 200000")
rp_s.check_error()

rp_s.tx_txt('CAN0:MODE LOOPBACK,ON')
print("CAN0:MODE LOOPBACK,ON")
rp_s.check_error()

rp_s.tx_txt('CAN0:START')
print("CAN0:START")
rp_s.check_error()

rp_s.tx_txt('CAN0:OPEN')
print("CAN0:OPEN")
rp_s.check_error()

rp_s.tx_txt('CAN0:Send123 1,2,3')
print("CAN0:SEND123 1,2,3")
rp_s.check_error()

rp_s.tx_txt('CAN0:Read:Timeout2000?')
print("CAN0:Read:Timeout2000?",rp_s.rx_txt_check_error())

rp_s.tx_txt('CAN0:CLOSE')
print("CAN0:CLOSE")
rp_s.check_error()