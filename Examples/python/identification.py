#!/usr/bin/env python3

import visa


# connect to the instrument
rm = visa.ResourceManager('@py')
#rm.list_resources()
rp = rm.open_resource('TCPIP::rp-f0508c.local::5000::SOCKET')


# read identification string
print(rp.query("*IDN?"))


# close instrument connection
rp.close()
