#!/usr/bin/python3

import numpy as np
from rp_overlay import overlay
import rp
import math

fpga = overlay()
rp.rp_Init()
#rp.rp_EnableDebugReg()
rp.rp_GenReset()

memory = rp.rp_AcqAxiGetMemoryRegion()
start_addr = memory[1]

rp.rp_GenAmp(rp.RP_CH_1, 1)
rp.rp_GenAmp(rp.RP_CH_2, 0.8)

rp.rp_GenMode(rp.RP_CH_1, rp.RP_GEN_MODE_BURST)
rp.rp_GenMode(rp.RP_CH_2, rp.RP_GEN_MODE_BURST)
rp.rp_GenBurstCount(rp.RP_CH_1, 1)
rp.rp_GenBurstCount(rp.RP_CH_2, 1)
rp.rp_GenBurstRepetitions(rp.RP_CH_1, 3);
rp.rp_GenBurstPeriod(rp.RP_CH_1, 500000);

cycles = 1 # how many sine cycles
resolution = 128 * 1024

length = np.pi * 2 * cycles

wave_ch1 = np.sin(np.arange(0, length, length / resolution,dtype=np.float32))
wave_ch2 = np.arange(0, length, length / resolution,dtype=np.float32)

for i in range(0, resolution):
    t = (2 * math.pi) / resolution * i
    b = math.fabs(math.sin(t) + 0.5)
    if(b >= 1):
        b = 1
    if( b<= -1):
        b = -1.0
    wave_ch1[i] = math.fabs(wave_ch1[i])
    wave_ch2[i] = b

# 1 sample of float -> 2 bytes int16
start_addr_ch1 = start_addr
end_addr_ch1 = start_addr + resolution * 2
start_addr_ch2 = start_addr + resolution * 2 + 4096
end_addr_ch2 = start_addr_ch2 + resolution * 2

rp.rp_GenAxiReserveMemory(rp.RP_CH_1,start_addr_ch1, end_addr_ch1)
rp.rp_GenAxiReserveMemory(rp.RP_CH_2,start_addr_ch2, end_addr_ch2)

rp.rp_GenAxiWriteWaveform(rp.RP_CH_1,wave_ch1)
rp.rp_GenAxiWriteWaveform(rp.RP_CH_2,wave_ch2)

rp.rp_GenAxiSetDecimationFactor(rp.RP_CH_1,1)
rp.rp_GenAxiSetDecimationFactor(rp.RP_CH_2,1)

rp.rp_GenAxiSetEnable(rp.RP_CH_1,True)
rp.rp_GenAxiSetEnable(rp.RP_CH_2,True)

rp.rp_GenTriggerSource(rp.RP_CH_1, rp.RP_GEN_TRIG_SRC_INTERNAL)

# Enable output synchronisation
rp.rp_GenOutEnableSync(True)
rp.rp_GenSynchronise()

rp.rp_GenAxiReleaseMemory(rp.RP_CH_1)
rp.rp_GenAxiReleaseMemory(rp.RP_CH_2)
