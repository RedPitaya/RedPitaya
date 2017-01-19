#!/usr/bin/python3

from redpitaya import *

for i in range(4):
    val = base.AIpinGetValue(i)
    print("Measured voltage on AI[%d] = {%f} V" % (i, val))
