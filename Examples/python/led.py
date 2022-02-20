#!/usr/bin/python

import sys
import time
import redpitaya_scpi as scpi

rp_s = scpi.scpi(sys.argv[1])

rp_s.tx_txt('LED:MMC OFF')
print("LED:MMC OFF")

rp_s.tx_txt('LED:MMC?')
print("LED:MMC state",rp_s.rx_txt())

rp_s.tx_txt('LED:HB OFF')
print("LED:HB OFF")

rp_s.tx_txt('LED:HB?')
print("LED:HB state",rp_s.rx_txt())

rp_s.tx_txt('LED:ETH OFF')
print("LED:ETH OFF")

rp_s.tx_txt('LED:ETH?')
print("LED:ETH state",rp_s.rx_txt())

time.sleep(10)

rp_s.tx_txt('LED:MMC ON')
print("LED:MMC ON")

rp_s.tx_txt('LED:HB ON')
print("LED:HB ON")

rp_s.tx_txt('LED:ETH ON')
print("LED:ETH ON")

rp_s.tx_txt('LED:MMC?')
print("LED:MMC state",rp_s.rx_txt())

rp_s.tx_txt('LED:HB?')
print("LED:HB state",rp_s.rx_txt())

rp_s.tx_txt('LED:ETH?')
print("LED:ETH state",rp_s.rx_txt())

