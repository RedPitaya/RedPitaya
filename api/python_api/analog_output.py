#!/usr/bin/python3

from redpitaya import *

values = [1.0, 0.5, 0.8, 0.3]
    
for i, v in enumerate(values):
    print("pin[%d] = %f" % (i, v))
    base.AOpinSetValue(i, v)
