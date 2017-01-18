#!/usr/bin/python3

from ctypes import *
from redpitaya import *

base.Init()

size = 4
buff = [c_float(0)]*size

for i in range(size):
	ret = base.AIpinGetValue(i, buff[i])
	print("Measured voltage on AI[%d] = {%f} V ret" % (i, buff[i].value))

base.Release()
