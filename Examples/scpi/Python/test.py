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
# common code
###############################################################################

error = 0

class test ():
    tests = 0
    error = 0

    def check_query (self, name: str, query: str, value: str = ""):
        # register test
        self.tests = self.tests + 1
        # send query
        response = rp.query(query)
        # check query response
        if (response != value):
            self.error = self.error + 1
            print ("Test ERROR: " + name + ":")
            print ("    expected: " + value)
            print ("    response: " + response)
        # check SCPI error status

    def check_write_read (self, name: str, write: str, read: str, value: str = ""):
        pass

    def report (self):
        print ("REPORT:")
        print ("    PASSED: " + str(self.tests - self.error))
        print ("    FAILED: " + str(             self.error))
t = test()

###############################################################################
# SCPI integrated commands
###############################################################################

# read identification string
t.check_query("IDN",     "*IDN?",           "REDPITAYA,INSTR2017,0,01-02")
# read SCPI standard version
t.check_query("VERSion", "SYSTem:VERSion?", "1999.0")

###############################################################################
# HWID
###############################################################################

t.check_query("HWID",  "HWIDentification:HWID?",  '#H1')
t.check_query("EFUSE", "HWIDentification:EFUSE?", '#H0')
t.check_query("DNA",   "HWIDentification:DNA?",   '#H11C0DCE4B59085C')
t.check_query("GITH",  "HWIDentification:GITH?",  '"3c08fd5a94977f8e0118d002520168ef650fa777"')

###############################################################################
# report
###############################################################################

t.report()

rp.close()
