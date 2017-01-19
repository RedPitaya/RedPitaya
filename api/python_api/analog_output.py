#!/usr/bin/python3

from redpitaya import *
import sys

base.Init()

values = [1,1,1,1]
if len(sys.argv) > 1:
    values = map(float, sys.argv[1:])

for i, v in enumerate(values):
    print("pin[%d] = %f" % (i, v))
    base.AOpinSetValue(i, v)

base.Release()
