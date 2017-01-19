#!/usr/bin/python3

from ctypes import *
from redpitaya import *
from time import sleep

base.Init()

base.AcqSetDecimation(8)
base.AcqSetTriggerLevel(CH_1, 100)
base.AcqStart()
base.AcqSetTriggerSrc(TRIG_SRC_EXT_PE)

while base.AcqGetTriggerState() != TRIG_STATE_TRIGGERED:
    pass
print('triggered')

size = base.AcqGetBufSize();
buff = base.AcqGetOldestDataV(CH_1, size)
for v in buff:
    print(v)

base.Release()
