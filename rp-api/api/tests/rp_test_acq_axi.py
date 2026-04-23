#!/usr/bin/python3

import rp
import time
import numpy as np

def init_rp():
    print("rp.rp_InitAdressess()")
    res = rp.rp_InitAdressess()
    print(res)

    print("rp.rp_Init()")
    res = rp.rp_Init()
    print(res)

def test_acq_axi_basic():
    print("rp.rp_AcqReset()")
    res = rp.rp_AcqReset()
    print(res)

    print("rp.rp_AcqAxiGetBufferFillState(rp.RP_CH_1)")
    res = rp.rp_AcqAxiGetBufferFillState(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqAxiSetDecimationFactor(1)")
    res = rp.rp_AcqAxiSetDecimationFactor(1)
    print(res)

    print("rp.rp_AcqAxiGetDecimationFactor()")
    res = rp.rp_AcqAxiGetDecimationFactor()
    print(res)

    print("rp.rp_AcqAxiSetDecimationFactorCh(rp.RP_CH_1, 1)")
    res = rp.rp_AcqAxiSetDecimationFactorCh(rp.RP_CH_1, 1)
    print(res)

    print("rp.rp_AcqAxiGetDecimationFactorCh(rp.RP_CH_1)")
    res = rp.rp_AcqAxiGetDecimationFactorCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqAxiSetTriggerDelay(rp.RP_CH_1,128)")
    res = rp.rp_AcqAxiSetTriggerDelay(rp.RP_CH_1,128)
    print(res)

    print("rp.rp_AcqAxiGetTriggerDelay(rp.RP_CH_1)")
    res = rp.rp_AcqAxiGetTriggerDelay(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqAxiGetMemoryRegion()")
    res = rp.rp_AcqAxiGetMemoryRegion()
    print(res)

def test_acq_axi_buffer():
    res = rp.rp_AcqAxiGetMemoryRegion()
    start = res[1] if len(res) > 1 else 0
    size = res[2] if len(res) > 2 else 0

    print("rp.rp_AcqAxiSetBufferSamples(rp.RP_CH_1,start,1000)")
    res = rp.rp_AcqAxiSetBufferSamples(rp.RP_CH_1,start,1000)
    print(res)

    print("rp.rp_AcqAxiSetBufferBytes(rp.RP_CH_1,start,size)")
    res = rp.rp_AcqAxiSetBufferBytes(rp.RP_CH_1,start,size)
    print(res)

def test_acq_axi_enable():
    print("rp.rp_AcqAxiEnable(rp.RP_CH_1,True)")
    res = rp.rp_AcqAxiEnable(rp.RP_CH_1,True)
    print(res)

    print("rp.rp_AcqStart()")
    res = rp.rp_AcqStart()
    print(res)

    time.sleep(1)

    print("rp.rp_AcqSetTriggerSrc(rp.RP_TRIG_SRC_NOW)")
    res = rp.rp_AcqSetTriggerSrc(rp.RP_TRIG_SRC_NOW)
    print(res)

    time.sleep(1)

    print("rp.rp_AcqStop()")
    res = rp.rp_AcqStop()
    print(res)

def test_acq_axi_pointers():
    print("rp.rp_AcqAxiGetWritePointer(rp.RP_CH_1)")
    res = rp.rp_AcqAxiGetWritePointer(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqAxiGetWritePointerAtTrig(rp.RP_CH_1)")
    res = rp.rp_AcqAxiGetWritePointerAtTrig(rp.RP_CH_1)
    print(res)
    trig = res[1] if len(res) > 1 else 0

    return trig

def test_acq_axi_data(trig=0):
    ibuff = rp.i16Buffer(128)
    print("rp.rp_AcqAxiGetDataRaw(rp.RP_CH_1,trig,128,ibuff.cast())")
    res = rp.rp_AcqAxiGetDataRaw(rp.RP_CH_1,trig, 128,ibuff.cast())
    print(res,ibuff[0],ibuff[1],ibuff[2],ibuff[3],ibuff[4])

    fbuff = rp.fBuffer(128)
    print("rp.rp_AcqAxiGetDataV(rp.RP_CH_1,trig,128,fbuff.cast())")
    res = rp.rp_AcqAxiGetDataV(rp.RP_CH_1,trig,128,fbuff.cast())
    print(res,fbuff[0],fbuff[1],fbuff[2],fbuff[3],fbuff[4])

def test_acq_axi_numpy():
    print("Testing numpy")
    N = 128
    arr_i16 = np.zeros(N, dtype=np.int16)
    arr_f = np.zeros(N, dtype=np.float32)

    print("rp.rp_AcqAxiGetDataRawNP(rp.RP_CH_1,0,arr_i16)")
    res = rp.rp_AcqAxiGetDataRawNP(rp.RP_CH_1,0,arr_i16)
    print(arr_i16)

    print("rp.rp_AcqAxiGetDataVNP(rp.RP_CH_1,0,arr_f)")
    res = rp.rp_AcqAxiGetDataVNP(rp.RP_CH_1,0,arr_f)
    print(arr_f)

    print("rp.rp_AcqAxiGetDataRawDirect(rp.RP_CH_1,0,128)")
    res = rp.rp_AcqAxiGetDataRawDirect(rp.RP_CH_1,0, 128)
    print(res)

    print("End testing numpy")

def test_acq_axi_offset():
    print("rp.rp_AcqAxiSetOffset(rp.RP_CH_1,0.5)")
    res = rp.rp_AcqAxiSetOffset(rp.RP_CH_1,0.5)
    print(res)

    print("rp.rp_AcqAxiGetOffset(rp.RP_CH_1)")
    res = rp.rp_AcqAxiGetOffset(rp.RP_CH_1)
    print(res)

if __name__ == "__main__":
    init_rp()
    test_acq_axi_basic()
    test_acq_axi_buffer()
    test_acq_axi_enable()
    trig = test_acq_axi_pointers()
    test_acq_axi_data(trig)
    test_acq_axi_numpy()
    test_acq_axi_offset()