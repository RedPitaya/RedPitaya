#!/usr/bin/python3

from redpitaya import *
from math import pi, sin

base.Init()

buf_size = base.AcqGetBufSize()

t = []
x = []
y = []
for i in range(buf_size):
    t.append((2*pi)/buf_size*i)
    x.append(sin(t[i]) + ((1.0/3.0) * sin(t[i] * 3)))
    y.append((1.0/2.0) * sin(t[i]) + (1.0/4.0) * sin(t[i] * 4))

base.GenWaveform(CH_1, WAVEFORM_ARBITRARY)
base.GenWaveform(CH_2, WAVEFORM_ARBITRARY)

base.GenArbWaveform(CH_1, x);
base.GenArbWaveform(CH_2, y);

base.GenAmp(CH_1, 0.7);
base.GenAmp(CH_2, 1.0);

base.GenFreq(CH_1, 4000.0);
base.GenFreq(CH_2, 4000.0);

base.GenOutEnable(CH_1);
base.GenOutEnable(CH_2);

base.Release()
