#!/usr/bin/python3

from ctypes import *
from redpitaya import *
from time import sleep

base.Init()

size = 4
buff = [c_float(0) for i in range(size)]

for i in range(size):
	ret = base.AIpinGetValue(i, buff[i])
	print("Measured voltage on AI[%d] = {%f} V ret" % (i, buff[i].value))
