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

for dev in range(2):
    # SCPI channels are indexed from 1 while API is indexed from 0
    ch = dev+1

    # data rate decimation 
    rp.write(":ACQuire{}:INPut:DECimation 1".format(ch))

    # trigger timing [sample periods]
    rp.write(":ACQuire{}:SAMPle:PRE ".format(ch)  + str(buffer_size//4 * 1))
    rp.write(":ACQuire{}:SAMPle:POST ".format(ch) + str(buffer_size//4 * 3))

    # define event synchronization source
    rp.write(":ACQuire1:EVENT:SYNChronization:SOURce OSC1")
    # use OSC1 as hardware trigger source
    rp.write(":ACQuire1:EVENT:TRIGger:SOURce OSC1")

# trigger level and slope
rp.write(":ACQuire1:TRIGger:LEVel  0.4,  0.5")
rp.write(":ACQuire1:TRIGger:SLOPe POSitive")
rp.write(":ACQuire2:TRIGger:LEVel -0.2, -0.15")
rp.write(":ACQuire2:TRIGger:SLOPe NEGative")

# synchronization source is the default, which is the module itself
# reset and start state machine
rp.write(":ACQuire1:RESET")
rp.write(":ACQuire1:START")
#rp.write(":ACQuire1:TRIGGER")

# wait for data
while (int(rp.query(":ACQuire1:RUN?"))):
    pass
print ('triggered')

# read back table data
values = [None, None]
for dev in range(2):
    # SCPI channels are indexed from 1 while API is indexed from 0
    ch = dev+1

    if args.bin:
        values[dev] = rp.query_binary_values(":ACQuire{}:TRACe:DATA:RAW? {}".format(ch, buffer_size), datatype='h')
    else:
        values[dev] = rp.query_ascii_values(":ACQuire{}:TRACe:DATA:DATA? {}".format(ch, buffer_size))

print(values)

import matplotlib.pyplot as plt
t = range(0, buffer_size)
plt.plot(t, values[0], t, values[1])
plt.show()

rp.close()
