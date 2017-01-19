#!/usr/bin/python3

from redpitaya import *
from time import sleep

base.Init()

base.GenReset()
base.GenFreq(CH_1, 20000.0)
base.GenAmp(CH_1, 1.0)
base.GenWaveform(CH_1, WAVEFORM_SINE)
base.GenOutEnable(CH_1)

base.AcqReset()
base.AcqSetDecimation(1)
base.AcqSetTriggerLevel(CH_1, 0.1)
base.AcqSetTriggerDelay(CH_1)

base.AcqStart()

sleep(1)
base.AcqSetTriggerSrc(TRIG_SRC_CHA_PE)

while base.AcqGetTriggerState() == TRIG_STATE_WAITING:
    pass
print('triggered')

size = base.AcqGetBufSize()
buff = base.AcqGetOldestDataV(CH_1, size);
for v in buff:
    print(v)

base.Release()
