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

# read GPIO
for i in range(16):
    value = rp.query(":GPIO:PIN? {}".format(i))
    print("get GPIO[{}] = {}".format(i, value))

# write GPIO
values = [0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1]
for i in range(16):
    value = values[i]
    rp.write(":GPIO:PIN {}, {}".format(i, value))
    print("set GPIO[{}] = {}".format(i, value))

rp.close()
