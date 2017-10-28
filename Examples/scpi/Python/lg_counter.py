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

# GPIO mode, all GPIO signals are connected to logic generator
rp.write(":MANAGement:GPIO:MODE #hFFFF")

# buffer waveform and sample timing
waveform = range(buffer_size)

if args.bin:
    rp.write_binary_values(":LG:TRACe:DATA:RAW ", waveform, datatype='h')
else:
    # TODO: due to a bug in PyVISA the documented use of write_ascii_values does not work
    #rp.write_ascii_values(":LG:TRACe:DATA:DATA", y_sum)
    rp.write(":LG:TRACe:DATA:DATA " + ','.join(map(str, waveform)))

# burst half the buffer with then idle for quarter buffer, repeat 4 times
rp.write(":LG:BURSt:DATA:REPetitions 2")
rp.write(":LG:BURSt:DATA:LENgth "   + str(buffer_size))
rp.write(":LG:BURSt:PERiod:LENgth " + str(buffer_size))
rp.write(":LG:BURSt:PERiod:NUMber INFinity")

# set output amplitude, offset and enable it
rp.write(":LG:OUTPut:ENABle " + str(0xffff)
                        + "," + str(0xffff))  # all pins have outputs enabled (for both output values 0/1)
rp.write(":LG:OUTPut:MASK "   + str(0x0000))  # all bits come from ASG, none are constants
rp.write(":LG:OUTPut:VALue "  + str(0x0000))  # the constant pin values are irrelevant since they are not used

# define event synchronization source
rp.write(":LG:EVENT:SYNChronization:SOURce LG")
# there are no HW trigger sources
rp.write(":LG:EVENT:TRIGger:SOURce NONE")

# reset, start, trigger state machine
rp.write(":LG:RESET")
rp.write(":LG:START")
rp.write(":LG:TRIGger")

rp.close()
