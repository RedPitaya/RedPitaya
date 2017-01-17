#!/usr/bin/python3

from ctypes import *
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

state = c_long(TRIG_STATE_TRIGGERED)
while True:
	ret = base.AcqGetTriggerState(state)
	if (state.value == TRIG_STATE_TRIGGERED):
		print("TRIGGERED")
		break

size = c_long(16384)
buff = misc.CreateFloatBuffer(size.value)
base.AcqGetOldestDataV(CH_1, size, buff);
for i in range(size.value):
	print(buff[i])

base.Release()
