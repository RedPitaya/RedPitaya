#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(description='SCPI test.')
parser.add_argument('adr',                                  help='provide IP address or URL')
parser.add_argument('-p', '--port', type=int, default=5000, help='specify SCPI port (default is 5000)')
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
print(rp.query("*IDN?"))
# read SCPI standard version
print(rp.query("SYSTem:VERSion?"))

rp.close()
