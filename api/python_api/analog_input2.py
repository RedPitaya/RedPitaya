#!/usr/bin/python3

from ctypes import *
from redpitaya import *
import sys

base.Init()

values = [float(v) for v in sys.argv[1:]]

for i, v in enumerate(values):
	print("pin[%d] = %f" % (i, v))
	base.AOpinSetValue(i, v)

base.Release()
