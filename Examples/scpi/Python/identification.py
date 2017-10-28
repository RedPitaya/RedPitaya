#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(description='SCPI: read identification.')
parser.add_argument('adr',          type=str, default='127.0.0.1', help='provide IP address or URL')
parser.add_argument('-p', '--port', type=int, default=5000,        help='specify SCPI port (default is 5000)')
parser.add_argument('-b', '--bin',  action="store_true",           help='use binary data transfer instead of the default ASCII')
parser.add_argument('--py',         action="store_true",           help='use PyVISA-py (by default the system visa library is used)')
args = parser.parse_args()

###############################################################################
# connect to the instrument
###############################################################################

import visa

rm = visa.ResourceManager('@py' if args.py else '')
#rm.list_resources()
rp = rm.open_resource('TCPIP::{}::{}::SOCKET'.format(args.adr, args.port), read_termination = '\r\n')

###############################################################################
# SCPI exchange
###############################################################################

# read identification string
print("Red Pitaya SCPI server ID = " + rp.query("*IDN?"))
# read SCPI standard version
print("SCPI standard version = " + rp.query("SYSTem:VERSion?"))

# HWID
print("Red Pitaya HWID: HWID  = " + rp.query("HWIDentification:HWID?"))
print("Red Pitaya HWID: EFUSE = " + rp.query("HWIDentification:EFUSE?"))
print("Red Pitaya HWID: DNA   = " + rp.query("HWIDentification:DNA?"))
print("Red Pitaya HWID: GITH  = " + rp.query("HWIDentification:GITH?"))

rp.close()
