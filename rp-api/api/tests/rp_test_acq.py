#!/usr/bin/python3

import rp
import time
import numpy as np

def init_rp():
    print("rp.rp_InitAddresses()")
    res = rp.rp_InitAddresses()
    print(res)

    print("rp.rp_Init()")
    res = rp.rp_Init()
    print(res)

def test_acquisition_basic():
    print("rp.rp_AcqResetFpga()")
    res = rp.rp_AcqResetFpga()
    print(res)

    print("rp.rp_AcqSetArmKeep(False)")
    res = rp.rp_AcqSetArmKeep(False)
    print(res)

    print("rp.rp_AcqGetArmKeep()")
    res = rp.rp_AcqGetArmKeep()
    print(res)

    print("rp.rp_AcqGetBufferFillState()")
    res = rp.rp_AcqGetBufferFillState()
    print(res)

    print("rp.rp_AcqSetDecimation(rp.RP_DEC_1)")
    res = rp.rp_AcqSetDecimation(rp.RP_DEC_1)
    print(res)

    print("rp.rp_AcqGetDecimation()")
    res = rp.rp_AcqGetDecimation()
    print(res)

def test_acquisition_decimation():
    print("rp.rp_AcqConvertFactorToDecimation(1)")
    res = rp.rp_AcqConvertFactorToDecimation(1)
    print(res)

    print("rp.rp_AcqSetDecimationFactor(2)")
    res = rp.rp_AcqSetDecimationFactor(2)
    print(res)

    print("rp.rp_AcqGetDecimationFactor()")
    res = rp.rp_AcqGetDecimationFactor()
    print(res)

    print("rp.rp_AcqGetSamplingRateHz()")
    res = rp.rp_AcqGetSamplingRateHz()
    print(res)

    print("rp.rp_AcqSetAveraging(False)")
    res = rp.rp_AcqSetAveraging(False)
    print(res)

    print("rp.rp_AcqGetAveraging()")
    res = rp.rp_AcqGetAveraging()
    print(res)

    print("rp.rp_AcqSetAveragingCh(rp.RP_CH_1,True)")
    res = rp.rp_AcqSetAveragingCh(rp.RP_CH_1,True)
    print(res)

    print("rp.rp_AcqGetAveragingCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetAveragingCh(rp.RP_CH_1)
    print(res)

def test_acquisition_filter():
    print("rp.rp_AcqSetBypassFilter(rp.RP_CH_1,False)")
    res = rp.rp_AcqSetBypassFilter(rp.RP_CH_1,False)
    print(res)

    print("rp.rp_AcqGetBypassFilter(rp.RP_CH_1)")
    res = rp.rp_AcqGetBypassFilter(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqSetCalibInFPGA(rp.RP_CH_1)")
    res = rp.rp_AcqSetCalibInFPGA(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetCalibInFPGA(rp.RP_CH_1)")
    res = rp.rp_AcqGetCalibInFPGA(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqUpdateAcqFilter(rp.RP_CH_1)")
    res = rp.rp_AcqUpdateAcqFilter(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetFilterCalibValue(rp.RP_CH_1)")
    res = rp.rp_AcqGetFilterCalibValue(rp.RP_CH_1)
    print(res)

def test_acquisition_trigger():
    print("rp.rp_AcqSetTriggerSrc(rp.RP_TRIG_SRC_DISABLED)")
    res = rp.rp_AcqSetTriggerSrc(rp.RP_TRIG_SRC_DISABLED)
    print(res)

    print("rp.rp_AcqGetTriggerSrc()")
    res = rp.rp_AcqGetTriggerSrc()
    print(res)

    print("rp.rp_AcqGetTriggerState()")
    res = rp.rp_AcqGetTriggerState()
    print(res)

    print("rp.rp_AcqSetTriggerDelay(100)")
    res = rp.rp_AcqSetTriggerDelay(100)
    print(res)

    print("rp.rp_AcqGetTriggerDelay()")
    res = rp.rp_AcqGetTriggerDelay()
    print(res)

    print("rp.rp_AcqSetTriggerDelayNs(1000)")
    res = rp.rp_AcqSetTriggerDelayNs(1000)
    print(res)

    print("rp.rp_AcqGetTriggerDelayNs()")
    res = rp.rp_AcqGetTriggerDelayNs()
    print(res)

    print("rp.rp_AcqSetTriggerDelayNsDirect(1000)")
    res = rp.rp_AcqSetTriggerDelayNsDirect(1000)
    print(res)

    print("rp.rp_AcqGetTriggerDelayNsDirect()")
    res = rp.rp_AcqGetTriggerDelayNsDirect()
    print(res)

def test_acquisition_trigger_level():
    print("rp.rp_AcqGetPreTriggerCounter()")
    res = rp.rp_AcqGetPreTriggerCounter()
    print(res)

    print("rp.rp_AcqSetTriggerLevel(rp.RP_T_CH_1,0.1)")
    res = rp.rp_AcqSetTriggerLevel(rp.RP_T_CH_1,0.1)
    print(res)

    print("rp.rp_AcqGetTriggerLevel(rp.RP_T_CH_1)")
    res = rp.rp_AcqGetTriggerLevel(rp.RP_T_CH_1)
    print(res)

    print("rp.rp_AcqSetTriggerHyst(0.01)")
    res = rp.rp_AcqSetTriggerHyst(0.01)
    print(res)

    print("rp.rp_AcqGetTriggerHyst()")
    res = rp.rp_AcqGetTriggerHyst()
    print(res)

def test_acquisition_gain():
    print("rp.rp_AcqSetGain(rp.RP_CH_1,rp.RP_HIGH)")
    res = rp.rp_AcqSetGain(rp.RP_CH_1,rp.RP_HIGH)
    print(res)

    print("rp.rp_AcqGetGain(rp.RP_CH_1)")
    res = rp.rp_AcqGetGain(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetGainV(rp.RP_CH_1)")
    res = rp.rp_AcqGetGainV(rp.RP_CH_1)
    print(res)

def test_acquisition_offset():
    print("rp.rp_AcqSetOffset(rp.RP_CH_1,0.5)")
    res = rp.rp_AcqSetOffset(rp.RP_CH_1,0.5)
    print(res)

    print("rp.rp_AcqGetOffset(rp.RP_CH_1)")
    res = rp.rp_AcqGetOffset(rp.RP_CH_1)
    print(res)

def test_acquisition_control():
    print("rp.rp_AcqGetWritePointer()")
    res = rp.rp_AcqGetWritePointer()
    print(res)

    print("rp.rp_AcqGetWritePointerAtTrig()")
    res = rp.rp_AcqGetWritePointerAtTrig()
    print(res)

    print("rp.rp_AcqReset()")
    res = rp.rp_AcqReset()
    print(res)

    print("rp.rp_AcqUnlockTrigger()")
    res = rp.rp_AcqUnlockTrigger()
    print(res)

    print("rp.rp_AcqGetUnlockTrigger()")
    res = rp.rp_AcqGetUnlockTrigger()
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

def test_acquisition_ext_trigger():
    print("rp.rp_AcqSetExtTriggerDebouncerUs(123)")
    res = rp.rp_AcqSetExtTriggerDebouncerUs(123)
    print(res)

    print("rp.rp_AcqGetExtTriggerDebouncerUs()")
    res = rp.rp_AcqGetExtTriggerDebouncerUs()
    print(res)

def test_acquisition_ac_dc():
    print("rp.rp_AcqSetAC_DC(rp.RP_CH_1,rp.RP_DC)")
    res = rp.rp_AcqSetAC_DC(rp.RP_CH_1,rp.RP_DC)
    print(res)

    print("rp.rp_AcqGetAC_DC(rp.RP_CH_1)")
    res = rp.rp_AcqGetAC_DC(rp.RP_CH_1)
    print(res)

def test_acquisition_data():
    print("rp.rp_AcqGetNormalizedDataPos(3423432)")
    res = rp.rp_AcqGetNormalizedDataPos(3423432)
    print(res)

    ibuff = rp.i16Buffer(1025)
    print("rp.rp_AcqGetDataPosRaw(rp.RP_CH_1,0,1024,ibuff.cast(),1025)")
    res = rp.rp_AcqGetDataPosRaw(rp.RP_CH_1,0,1024,ibuff.cast(),1025)
    print(res,ibuff[0],ibuff[1],ibuff[2],ibuff[3],ibuff[4])

    fbuff = rp.fBuffer(1025)
    print("rp.rp_AcqGetDataPosV(rp.RP_CH_1,0,1024,fbuff,1025)")
    res = rp.rp_AcqGetDataPosV(rp.RP_CH_1,0,1024,fbuff,1025)
    print(res,fbuff[0],fbuff[1],fbuff[2],fbuff[3],fbuff[4])

def test_acquisition_numpy():
    print("Testing numpy")
    N = 128
    arr_i16 = np.zeros(N, dtype=np.int16)
    arr_f = np.zeros(N, dtype=np.float32)

    print("rp.rp_AcqGetDataPosRawNP(rp.RP_CH_1,0,128,arr_i16)")
    res = rp.rp_AcqGetDataPosRawNP(rp.RP_CH_1,0,128,arr_i16)
    print(arr_i16)

    print("rp.rp_AcqGetDataPosVNP(rp.RP_CH_1,0,128,arr_f)")
    res = rp.rp_AcqGetDataPosVNP(rp.RP_CH_1,0,128,arr_f)
    print(arr_f)

    print("rp.rp_AcqGetDataRawNP(rp.RP_CH_1,0,arr_i16)")
    res = rp.rp_AcqGetDataRawNP(rp.RP_CH_1,0,arr_i16)
    print(arr_i16)

    print("rp.rp_AcqGetDataRawWithCalibNP(rp.RP_CH_1,0,arr_i16)")
    res = rp.rp_AcqGetDataRawWithCalibNP(rp.RP_CH_1,0,arr_i16)
    print(arr_i16)

    print("rp.rp_AcqGetDataVNP(rp.RP_CH_1,0,arr_f)")
    res = rp.rp_AcqGetDataVNP(rp.RP_CH_1,0,arr_f)
    print(arr_f)

    print("rp.rp_AcqGetOldestDataRawNP(rp.RP_CH_1,arr_i16)")
    res = rp.rp_AcqGetOldestDataRawNP(rp.RP_CH_1,arr_i16)
    print(arr_i16)

    print("rp.rp_AcqGetLatestDataRawNP(rp.RP_CH_1,arr_i16)")
    res = rp.rp_AcqGetLatestDataRawNP(rp.RP_CH_1,arr_i16)
    print(arr_i16)

    print("rp.rp_AcqGetOldestDataVNP(rp.RP_CH_1,arr_f)")
    res = rp.rp_AcqGetOldestDataVNP(rp.RP_CH_1,arr_f)
    print(arr_f)

    print("rp.rp_AcqGetLatestDataVNP(rp.RP_CH_1,arr_f)")
    res = rp.rp_AcqGetLatestDataVNP(rp.RP_CH_1,arr_f)
    print(arr_f)

    print("End testing numpy")

def test_acquisition_data_advanced():
    ibuff = rp.i16Buffer(1024)
    print("rp.rp_AcqGetDataRaw(rp.RP_CH_1,0,1024,ibuff.cast())")
    res = rp.rp_AcqGetDataRaw(rp.RP_CH_1,0,1024,ibuff.cast())
    print(res,ibuff[0],ibuff[1],ibuff[2],ibuff[3],ibuff[4])

    ibuff = rp.i16Buffer(1024)
    print("rp.rp_AcqGetDataRawWithCalib(rp.RP_CH_1,0,1024,ibuff.cast())")
    res = rp.rp_AcqGetDataRawWithCalib(rp.RP_CH_1,0,1024,ibuff.cast())
    print(res,ibuff[0],ibuff[1],ibuff[2],ibuff[3],ibuff[4])

    ibuff = rp.i16Buffer(1024)
    print("rp.rp_AcqGetOldestDataRaw(rp.RP_CH_1,1024,ibuff.cast())")
    res = rp.rp_AcqGetOldestDataRaw(rp.RP_CH_1,1024,ibuff.cast())
    print(res,ibuff[0],ibuff[1],ibuff[2],ibuff[3],ibuff[4])

    ibuff = rp.i16Buffer(1024)
    print("rp.rp_AcqGetLatestDataRaw(rp.RP_CH_1,1024,ibuff.cast())")
    res = rp.rp_AcqGetLatestDataRaw(rp.RP_CH_1,1024,ibuff.cast())
    print(res,ibuff[0],ibuff[1],ibuff[2],ibuff[3],ibuff[4])

    fbuff = rp.fBuffer(1025)
    print("rp.rp_AcqGetDataV(rp.RP_CH_1,0,1024,fbuff)")
    res = rp.rp_AcqGetDataV(rp.RP_CH_1,0,1024,fbuff)
    print(res,fbuff[0],fbuff[1],fbuff[2],fbuff[3],fbuff[4])

    fbuff = rp.fBuffer(1024)
    print("rp.rp_AcqGetOldestDataV(rp.RP_CH_1,1024,fbuff.cast())")
    res = rp.rp_AcqGetOldestDataV(rp.RP_CH_1,1024,fbuff.cast())
    print(res,fbuff[0],fbuff[1],fbuff[2],fbuff[3],fbuff[4])

    fbuff = rp.fBuffer(1024)
    print("rp.rp_AcqGetLatestDataV(rp.RP_CH_1,1024,fbuff.cast())")
    res = rp.rp_AcqGetLatestDataV(rp.RP_CH_1,1024,fbuff.cast())
    print(res,fbuff[0],fbuff[1],fbuff[2],fbuff[3],fbuff[4])

def test_acquisition_buffer():
    print("rp.rp_AcqGetBufSize()")
    res = rp.rp_AcqGetBufSize()
    print(res)

    print("rp.rp_createBuffer(2,1024,True,False,False)")
    buff_t = rp.rp_createBuffer(2,1024,True,False,False)
    print(buff_t)

    print("rp.rp_AcqGetData(0,buff_t)")
    res = rp.rp_AcqGetData(0,buff_t)
    print(res)
    print(buff_t.size)
    print(buff_t.channels)

    print("rp.rp_AcqGetDataWithCorrection(0,10,-5,buff_t)")
    res = rp.rp_AcqGetDataWithCorrection(0,10,-5,buff_t)
    print(res)

    print("rp.rp_deleteBuffer(buff_t)")
    res = rp.rp_deleteBuffer(buff_t)
    print(res)

    print("rp.rp_createBuffer(2,1024,False,False,True)")
    buff_t = rp.rp_createBuffer(2,1024,False,False,True)
    print(buff_t)

    print("rp.rp_AcqGetData(0,buff_t)")
    res = rp.rp_AcqGetData(0,buff_t)
    print(res)

    print("rp.rp_deleteBuffer(buff_t)")
    res = rp.rp_deleteBuffer(buff_t)
    print(res)

def test_interrupts():
    print("Testing interrupt functions")

    print("rp.rp_AcqIntTriggerRead(1000)")
    res = rp.rp_AcqIntTriggerRead(1000)
    print(res)

    print("rp.rp_AcqIntFillRead(1000)")
    res = rp.rp_AcqIntFillRead(1000)
    print(res)

    print("rp.rp_AcqIntTriggerReadCh(rp.RP_CH_1, 1000)")
    res = rp.rp_AcqIntTriggerReadCh(rp.RP_CH_1, 1000)
    print(res)

    print("rp.rp_AcqIntFillReadCh(rp.RP_CH_1, 1000)")
    res = rp.rp_AcqIntFillReadCh(rp.RP_CH_1, 1000)
    print(res)

def test_timestamp():
    print("Testing timestamp functions")

    print("rp.rp_AcqSetInitTimestamp(1000)")
    res = rp.rp_AcqSetInitTimestamp(1000)
    print(res)

    print("rp.rp_AcqGetTimestamp(rp.RP_CH_1)")
    res = rp.rp_AcqGetTimestamp(rp.RP_CH_1)
    print(res)

def test_16bit_mode():
    print("Testing 16-bit mode functions")

    print("rp.rp_AcqSet16BitMode(True)")
    res = rp.rp_AcqSet16BitMode(True)
    print(res)

    print("rp.rp_AcqGet16BitMode()")
    res = rp.rp_AcqGet16BitMode()
    print(res)

def test_waveform_data():
    print("Testing waveform data functions")

    print("rp.rp_GetWaveformDataV(rp.RP_CH_1)")
    res = rp.rp_GetWaveformDataV(rp.RP_CH_1)
    print(res)

def test_acquisition_int_mask():
    print("Testing interrupt mask functions")

    print("rp.rp_AcqSetIntMask(rp.RP_INT_MODE_TRIGGER, True)")
    res = rp.rp_AcqSetIntMask(rp.RP_INT_MODE_TRIGGER, True)
    print(res)

    print("rp.rp_AcqGetIntMask(rp.RP_INT_MODE_TRIGGER)")
    res = rp.rp_AcqGetIntMask(rp.RP_INT_MODE_TRIGGER)
    print(res)

    print("rp.rp_AcqSetIntMask(rp.RP_INT_MODE_FILL, False)")
    res = rp.rp_AcqSetIntMask(rp.RP_INT_MODE_FILL, False)
    print(res)

    print("rp.rp_AcqGetIntMask(rp.RP_INT_MODE_FILL)")
    res = rp.rp_AcqGetIntMask(rp.RP_INT_MODE_FILL)
    print(res)

    print("rp.rp_AcqSetIntMaskCh(rp.RP_CH_1, rp.RP_INT_MODE_TRIGGER, True)")
    res = rp.rp_AcqSetIntMaskCh(rp.RP_CH_1, rp.RP_INT_MODE_TRIGGER, True)
    print(res)

    print("rp.rp_AcqGetIntMaskCh(rp.RP_CH_1, rp.RP_INT_MODE_TRIGGER)")
    res = rp.rp_AcqGetIntMaskCh(rp.RP_CH_1, rp.RP_INT_MODE_TRIGGER)
    print(res)

    print("rp.rp_AcqSetIntMaskCh(rp.RP_CH_2, rp.RP_INT_MODE_FILL, True)")
    res = rp.rp_AcqSetIntMaskCh(rp.RP_CH_2, rp.RP_INT_MODE_FILL, True)
    print(res)

    print("rp.rp_AcqGetIntMaskCh(rp.RP_CH_2, rp.RP_INT_MODE_FILL)")
    res = rp.rp_AcqGetIntMaskCh(rp.RP_CH_2, rp.RP_INT_MODE_FILL)
    print(res)

if __name__ == "__main__":
    init_rp()
    test_acquisition_basic()
    test_acquisition_decimation()
    test_acquisition_filter()
    test_acquisition_trigger()
    test_acquisition_trigger_level()
    test_acquisition_gain()
    test_acquisition_offset()
    test_acquisition_control()
    test_acquisition_ext_trigger()
    test_acquisition_ac_dc()
    test_acquisition_data()
    test_acquisition_numpy()
    test_acquisition_data_advanced()
    test_acquisition_buffer()
    test_interrupts()
    test_timestamp()
    test_16bit_mode()
    test_waveform_data()
    test_acquisition_int_mask()