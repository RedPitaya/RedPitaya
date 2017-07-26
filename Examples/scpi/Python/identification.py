#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(description='SCPI: read identification.')
parser.add_argument('adr',          type=str, default='127.0.0.1', help='provide IP address or URL')
parser.add_argument('-p', '--port', type=int, default=5000,        help='specify SCPI port (default is 5000)')
args = parser.parse_args()

###############################################################################
# connect to the instrument
###############################################################################

import visa

rm = visa.ResourceManager('@py')
#rm.list_resources()
rp = rm.open_resource('TCPIP::{}::{}::SOCKET'.format(args.adr, args.port), read_termination = '\r\n')

###############################################################################
# SCPI exchange
###############################################################################

# read identification string
print("Red Pitaya SCPI server ID = " + rp.query("*IDN?"))
# read SCPI standard version
print("SCPI standard version = " + rp.query("SYSTem:VERSion?"))

rp.close()
