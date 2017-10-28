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

    def check_query (self, name: str, query: str, read: str = "", expect = True):
        # register test
        self.tests = self.tests + 1
        # send query
        value = rp.query(query)
        # check query response
        if (value != read):
            print ("Test \"" + name + "\" ERROR:")
            print ("    expected: " + read)
            print ("    response: " + value)
            self.error = self.error + 1
        # check SCPI error status
        self.check_errors(name)

    def check_write_query (self, name: str, write: str, query: str, read: str = "", expect = True):
        # register test
        self.tests = self.tests + 1
        # send write/read
        rp.write(write)
        value = rp.query(query)
        # check read response
        if (value != read):
            print ("Test \"" + name + "\" value ERROR:")
            print ("    expected: " + read)
            print ("    response: " + value)
            self.error = self.error + 1
        # check SCPI error status
        self.check_errors(name)

    def check_errors (self, name: str):
        err_cnt = int(rp.query(":SYSTem:ERRor:COUNt?"))
        if (err_cnt):
            print ("Test \"" + name + "\" SCPI ERROR (cnt = " + str(err_cnt) + "):")
        for err in range(err_cnt):
            value = rp.query(":SYSTem:ERRor:NEXT?")
            print ("    SCPI ERR " + str(err) + ": " + value)
            self.error = self.error + 1

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
# MGMT
###############################################################################

# GPIO mode
values = [["#H0", True], ["#HFFFF", True], ["#H1FFFF", False]]
for idx, val in enumerate(values):
    t.check_write_query("gpio_mode " + str(idx), ":MANAGement:GPIO:MODE " + val[0], ":MANAGement:GPIO:MODE?", val[0], val[1])

# digital loop from generator to oscilloscope


#    // Management
#    {.pattern = ":MANAGement:GPIO[:MODE]",                   .callback = rpscpi_mgmt_set_gpio_mode,              },
#    {.pattern = ":MANAGement:GPIO[:MODE]?",                  .callback = rpscpi_mgmt_get_gpio_mode,              },
#    {.pattern = ":MANAGement:LOOP",                          .callback = rpscpi_mgmt_set_loop,                   },
#    {.pattern = ":MANAGement:LOOP?",                         .callback = rpscpi_mgmt_get_loop,                   },
#    {.pattern = ":MANAGement:PRINT",                         .callback = rpscpi_mgmt_print,                      },  // debug command


###############################################################################
# report
###############################################################################

t.report()

rp.close()
