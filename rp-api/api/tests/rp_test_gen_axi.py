#!/usr/bin/python3

import rp
import numpy as np

def init_rp():
    print("rp.rp_InitAddresses()")
    res = rp.rp_InitAddresses()
    print(res)

    print("rp.rp_Init()")
    res = rp.rp_Init()
    print(res)

def test_asg_axi_memory():
    print("rp.rp_GenAxiGetMemoryRegion()")
    res = rp.rp_GenAxiGetMemoryRegion()
    print(res)

def test_asg_axi_enable():
    print("rp.rp_GenAxiSetEnable(rp.RP_CH_1, True)")
    res = rp.rp_GenAxiSetEnable(rp.RP_CH_1, True)
    print(res)

    print("rp.rp_GenAxiGetEnable(rp.RP_CH_1)")
    res = rp.rp_GenAxiGetEnable(rp.RP_CH_1)
    print(res)

def test_asg_axi_memory_reserve():
    print("rp.rp_GenAxiReserveMemory(rp.RP_CH_1, 0x1000, 0x2000)")
    res = rp.rp_GenAxiReserveMemory(rp.RP_CH_1, 0x1000, 0x2000)
    print(res)

    print("rp.rp_GenAxiReleaseMemory(rp.RP_CH_1)")
    res = rp.rp_GenAxiReleaseMemory(rp.RP_CH_1)
    print(res)

def test_asg_axi_decimation():
    print("rp.rp_GenAxiSetDecimationFactor(rp.RP_CH_1, 1)")
    res = rp.rp_GenAxiSetDecimationFactor(rp.RP_CH_1, 1)
    print(res)

    print("rp.rp_GenAxiGetDecimationFactor(rp.RP_CH_1)")
    res = rp.rp_GenAxiGetDecimationFactor(rp.RP_CH_1)
    print(res)

def test_asg_axi_waveform():
    N = 1024
    arr_f = np.zeros(N, dtype=np.float32)
    for n in range(N):
        arr_f[n] = n / N * 2 - 1

    print("rp.rp_GenAxiWriteWaveform(rp.RP_CH_1, arr_f)")
    res = rp.rp_GenAxiWriteWaveform(rp.RP_CH_1, arr_f)
    print(res)

    print("rp.rp_GenAxiWriteWaveformOffset(rp.RP_CH_1, 0, arr_f)")
    res = rp.rp_GenAxiWriteWaveformOffset(rp.RP_CH_1, 0, arr_f)
    print(res)

if __name__ == "__main__":
    init_rp()
    test_asg_axi_memory()
    test_asg_axi_enable()
    test_asg_axi_memory_reserve()
    test_asg_axi_decimation()
    test_asg_axi_waveform()