#!/usr/bin/python3

import time
import numpy as np
from rp_overlay import overlay
import rp

fpga = overlay()
rp.rp_Init()

channel = rp.RP_CH_1
channel2 = rp.RP_CH_2
waveform = rp.RP_WAVEFORM_SINE
freq = 100000
ampl = 1.0

# Acquisition paramters
dec = rp.RP_DEC_1

trig_lvl = 0.5
trig_dly = 0

acq_trig_sour = rp.RP_TRIG_SRC_CHA_PE
N = 128

rp.rp_GenReset()
rp.rp_AcqReset()

###### Generation #####
# OUT1
print("Gen_start")
rp.rp_GenWaveform(channel, waveform)
rp.rp_GenFreqDirect(channel, freq)
rp.rp_GenAmp(channel, ampl)

# OUT2
rp.rp_GenWaveform(channel2, waveform)
rp.rp_GenFreqDirect(channel2, freq)
rp.rp_GenAmp(channel2, ampl)

# Specify generator trigger source
#rp.rp_GenTriggerSource(channel, rp.RP_GEN_TRIG_SRC_INTERNAL)

# Enable output synchronisation
rp.rp_GenOutEnableSync(True)
rp.rp_GenTriggerOnly(channel)

##### Acquisition #####
# Set Decimation
rp.rp_AcqSetDecimation(rp.RP_DEC_1)

# Set trigger level and delay
rp.rp_AcqSetTriggerLevel(rp.RP_T_CH_1, trig_lvl)
rp.rp_AcqSetTriggerDelay(trig_dly)

# Start Acquisition
print("Acq_start")
rp.rp_AcqStart()
time.sleep(0.1)
# Specify trigger - input 1 positive edge
rp.rp_AcqSetTriggerSrc(acq_trig_sour)
time.sleep(0.1)

rp.rp_GenTriggerOnly(channel)       # Trigger generator

# Trigger state
while 1:
    trig_state = rp.rp_AcqGetTriggerState()[1]
    if trig_state == rp.RP_TRIG_STATE_TRIGGERED:
        break

# Fill state
while 1:
    if rp.rp_AcqGetBufferFillState()[1]:
        break

print("ACQ get data")
tp=rp.rp_AcqGetWritePointerAtTrig()[1]
# Get data
# RAW
arr_i16 = np.zeros(N, dtype=np.int16)
arr_f = np.zeros(N, dtype=np.float32)
res = rp.rp_AcqGetDataRawNP(rp.RP_CH_1,tp, arr_i16)
# Volts
res = rp.rp_AcqGetDataVNP(rp.RP_CH_1,tp, arr_f)
print(arr_i16)
print(arr_f)

