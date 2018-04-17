#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(description='SCPI: generate arbitrary signal.')
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

import numpy as np

# prepare custom waveform
length = 2**14
x  = np.linspace(0, 6, length) # generates x axis in range 0 to 6 with 20000 points
y1 = 0.8*np.sin(x) # the first sinus signal with the amplitude 0.8
y2 = 0.2*np.sin(21*x) # the second sinus signal with a frequency 20 times higher than the first one and the amplitude of 0.2
y_sum = y1+y2
bitsize=13

#visa.util.to_ieee_block(y_sum, 'f', is_big_endian)

# set output amplitude and offset
rp.write(":SOURce1:VOLTage:IMMediate:AMPlitude 1")
rp.write(":SOURce1:VOLTage:IMMediate:OFFSet 0")

# specify peridic mode, sinusoidal waveform and 1kHZ frequency
rp.write(":SOURce1:MODE PERiodic")
if args.bin:
    values = np.array(y_sum * (2**bitsize), dtype=np.int16)
    rp.write_binary_values(":SOURce1:TRACe:DATA:RAW ", values, datatype='h')
else:
    # TODO: due to a bug in PyVISA the documented use of write_ascii_values does not work
    #rp.write_ascii_values(":SOURce1:TRACe:DATA:DATA", y_sum)
    rp.write(":SOURce1:TRACe:DATA:DATA " + ','.join(map(str, y_sum)))
rp.write(":SOURce1:FREQuency:FIXed 1000")

# reset and start state machine
rp.write(":SOURce1:RESET")
rp.write(":SOURce1:START")

# enable output
rp.write(":OUTPUT1:STATe ON")

# trigger state machine
rp.write(":SOURce1:TRIGger")

# read back table data
if args.bin:
    values = rp.query_binary_values(":SOURce1:TRACe:DATA:RAW? {}".format(length), datatype='h')
else:
    values = rp.query_ascii_values(":SOURce1:TRACe:DATA:DATA? {}".format(length))
print(values)

rp.close()
