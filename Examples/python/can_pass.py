#!/usr/bin/python3

import sys
import time
import redpitaya_scpi as scpi

#rp_s = scpi.scpi(sys.argv[1])
rp_s = scpi.scpi("200.0.0.37")

rp_s.tx_txt('CAN:FPGA ON')
print("CAN:FPGA ON")
rp_s.check_error()

rp_s.tx_txt('CAN0:STOP')
print("CAN0:START")
rp_s.check_error()

rp_s.tx_txt('CAN1:STOP')
print("CAN1:STOP")
rp_s.check_error()

rp_s.tx_txt('CAN0:BITRate 200000')
print("CAN0:BitRate 200000")
rp_s.check_error()

rp_s.tx_txt('CAN1:BITR 200000')
print("CAN1:BitRate 200000")
rp_s.check_error()

rp_s.tx_txt('CAN0:MODE LOOPBACK,OFF')
print("CAN0:MODE LOOPBACK,OFF")
rp_s.check_error()

rp_s.tx_txt('CAN1:MODE LOOPBACK,OFF')
print("CAN1:MODE LOOPBACK,OFF")
rp_s.check_error()

rp_s.tx_txt('CAN0:START')
print("CAN0:START")
rp_s.check_error()

rp_s.tx_txt('CAN1:START')
print("CAN1:START")
rp_s.check_error()

rp_s.tx_txt('CAN0:OPEN')
print("CAN0:OPEN")
rp_s.check_error()

rp_s.tx_txt('CAN1:OPEN')
print("CAN1:OPEN")
rp_s.check_error()

rp_s.tx_txt('CAN0:Send123 1,2,3')
print("CAN0:SEND123 1,2,3")
rp_s.check_error()

rp_s.tx_txt('CAN0:Send321:E 1,2,3,4,5')
print("CAN0:Send321:E 1,2,3,4,5")
rp_s.check_error()

rp_s.tx_txt('CAN1:READ:TIMEOUT2000?')
print("CAN1:READ:TIMEOUT2000??",rp_s.rx_txt_check_error())

rp_s.tx_txt('CAN1:READ?')
print("CAN1:READ?",rp_s.rx_txt_check_error())

rp_s.tx_txt('CAN0:CLOSE')
print("CAN0:CLOSE")
rp_s.check_error()

rp_s.tx_txt('CAN1:CLOSE')
print("CAN1:CLOSE")
rp_s.check_error()
