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

# set output amplitude and offset
rp.write(":SOURce1:VOLTage:IMMediate:AMPlitude 1")
rp.write(":SOURce1:VOLTage:IMMediate:OFFSet 0")

# specify peridic mode, sinusoidal waveform and 1kHZ frequency
rp.write(":SOURce1:MODE PERiodic")
rp.write(":SOURce1:FUNCtion:SHAPe SINusoid")
rp.write(":SOURce1:FREQuency:FIXed 1000")

# reset and start state machine
rp.write(":SOURce1:RESET")
rp.write(":SOURce1:START")

# enable output
rp.write(":OUTPUT1:STATe ON")

# trigger state machine
rp.write(":SOURce1:TRIGger")

rp.close()
