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

import numpy as np

buffer_size = 2**14
bitsize = 13

# set output amplitude and offset
rp.write(":SOURce1:VOLTage:IMMediate:AMPlitude 1")
rp.write(":SOURce1:VOLTage:IMMediate:OFFSet 0")

# specify waveform points
waveform = np.array([
 0.00000,  0.48479,  0.85198,  0.99865,  0.87470,  0.50312, -0.01908, -0.54248,
-0.90647, -0.98961, -0.75350, -0.26397,  0.32157,  0.80137,  0.99859,  0.82977,
 0.34524, -0.27847, -0.79836, -0.99898, -0.78550, -0.23480,  0.42399,  0.90004,
 0.96970,  0.58720, -0.07926, -0.71329, -0.99766, -0.77674, -0.15008,  0.56173,
 0.97543,  0.85355,  0.24941, -0.50312, -0.96673, -0.85615, -0.22307,  0.55340,
 0.98570,  0.78612,  0.06924, -0.69908, -0.99739, -0.60732,  0.21327,  0.88655,
 0.91793,  0.26881, -0.58720, -0.99843, -0.63407,  0.23480,  0.91990,  0.85667,
 0.08126, -0.76718, -0.96283, -0.32157,  0.60732,  0.99642,  0], dtype=np.float32)

# program waveform
if args.bin:
    values = np.array(waveform * (2**bitsize), dtype=np.int16)
    rp.write_binary_values(":SOURce1:TRACe:DATA:RAW ", values, datatype='h')
else:
    # TODO: due to a bug in PyVISA the documented use of write_ascii_values does not work
    #rp.write_ascii_values(":SOURce1:TRACe:DATA:DATA", waveform)
    rp.write(":SOURce1:TRACe:DATA:DATA " + ','.join(map(str, waveform)))

# specify peridic mode, sinusoidal waveform
rp.write(":SOURce1:MODE BURSt")
rp.write(":SOURce1:BURSt:MODE FINite")

# burst half the buffer with then idle for quarter buffer, repeat 4 times
rp.write(":SOURce1:BURSt:DATA:REPetitions 1")
rp.write(":SOURce1:BURSt:DATA:LENgth "   + str(len(waveform)))
rp.write(":SOURce1:BURSt:PERiod:LENgth " + str(len(waveform)))
rp.write(":SOURce1:BURSt:PERiod:NUMber 1")

# reset and start state machine
rp.write(":SOURce1:RESET")
rp.write(":SOURce1:START")

# enable output
rp.write(":OUTPUT1:STATe ON")

# configure logic analyzer trigger source
# inputs enabled by given mask will reach on rising edge (positive)
rp.write(":LA:INPut:MASK #Hffff")
rp.write(":LA:TRIGger:PATTern:MASK #H0000")
rp.write(":LA:TRIGger:EDGE:POSitive #H0001")
rp.write(":LA:TRIGger:EDGE:NEGative #H0000")
rp.write(":SOURce1:EVENT:TRIGger:SOURce LA")

# trigger state machine
#rp.write(":SOURce1:TRIGger")

rp.close()
