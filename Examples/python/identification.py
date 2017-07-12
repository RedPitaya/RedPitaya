#!/usr/bin/env python3

import visa
rm = visa.ResourceManager('@py')
#rm.list_resources()
rp = rm.open_resource('TCPIP::rp-f0508c.local::5000::SOCKET', read_termination = '\r\n')
print(rp.query("*IDN?"))
rp.close()
