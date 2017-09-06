#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(description='SCPI: generate sinusoidal signal.')
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

buffer_size = 2**14

# set output amplitude and offset
rp.write(":SOURce1:VOLTage:IMMediate:AMPlitude 1")
rp.write(":SOURce1:VOLTage:IMMediate:OFFSet 0")

# specify burst mode, sinusoidal waveform
rp.write(":SOURce1:MODE BURSt")
rp.write(":SOURce1:FUNCtion:SHAPe SINusoid")

# burst half the buffer with then idle for quarter buffer, repeat 4 times
rp.write(":SOURce1:BURSt:DATA:REPetitions 1")
rp.write(":SOURce1:BURSt:DATA:LENgth "   + str(1 * buffer_size // 2))
rp.write(":SOURce1:BURSt:PERiod:LENgth " + str(3 * buffer_size // 4))
rp.write(":SOURce1:BURSt:PERiod:NUMber 4")
#rp.write(":SOURce1:BURSt:PERiod:NUMber INFinity")

# reset and start state machine
rp.write(":SOURce1:RESET")
rp.write(":SOURce1:START")

# enable output
rp.write(":OUTPUT1:STATe ON")

# trigger state machine
rp.write(":SOURce1:TRIGger")

rp.close()
