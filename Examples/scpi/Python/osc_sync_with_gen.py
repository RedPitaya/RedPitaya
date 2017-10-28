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

###############################################################################
# generator setting
###############################################################################

# specify burst mode, sinusoidal waveform
rp.write(":SOURce1:MODE BURSt")
rp.write(":SOURce1:FUNCtion:SHAPe SINusoid")

# burst half the buffer with then idle for quarter buffer, repeat 4 times
rp.write(":SOURce1:BURSt:DATA:REPetitions 1")
rp.write(":SOURce1:BURSt:DATA:LENgth "   + str(1 * buffer_size // 2))
rp.write(":SOURce1:BURSt:PERiod:LENgth " + str(3 * buffer_size // 4))
rp.write(":SOURce1:BURSt:PERiod:NUMber 4")
#rp.write(":SOURce1:BURSt:PERiod:NUMber INFinity")

# set output amplitude and offset
rp.write(":SOURce1:VOLTage:IMMediate:AMPlitude 1")
rp.write(":SOURce1:VOLTage:IMMediate:OFFSet 0")
# enable output
rp.write(":OUTPUT1:STATe ON")

# define event synchronization source
rp.write(":SOURce1:EVENT:SYNChronization:SOURce GEN1")

###############################################################################
# oscilloscope setting
###############################################################################

# data rate decimation 
rp.write(":ACQuire1:INPut:DECimation 4")

# trigger timing [sample periods]
rp.write(":ACQuire1:SAMPle:PRE "  + str(0))
rp.write(":ACQuire1:SAMPle:POST " + str(buffer_size))

# define event synchronization source
rp.write(":ACQuire1:EVENT:SYNChronization:SOURce GEN1")
# there are no HW trigger sources
rp.write(":ACQuire1:EVENT:TRIGger:SOURce NONE")

###############################################################################
# start measurement
###############################################################################

# reset and start state machine
rp.write(":SOURce1:RESET")
rp.write(":SOURce1:START")
# trigger state machine
rp.write(":SOURce1:TRIGger")

# wait for data
while (int(rp.query(":ACQuire1:RUN?"))):
    pass
print ('triggered')

# read back table data
if args.bin:
    values = rp.query_binary_values(":ACQuire1:TRACe:DATA:RAW? {}".format(buffer_size), datatype='h')
else:
    values = rp.query_ascii_values(":ACQuire1:TRACe:DATA:DATA? {}".format(buffer_size))
print(values)

import matplotlib.pyplot as plt
plt.plot(values)
plt.show()

rp.close()
