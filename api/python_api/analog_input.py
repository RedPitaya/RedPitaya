#!/usr/bin/python3

from ctypes import *
from redpitaya import *

base.Init()

for i in range(4):
    val = base.AIpinGetValue(i)
    print("Measured voltage on AI[%d] = {%f} V" % (i, val))

base.Release()
