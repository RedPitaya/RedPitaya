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

def test_generator_basic():
    print("rp.rp_GenReset()")
    res = rp.rp_GenReset()
    print(res)

    print("rp.rp_GenOutEnable(rp.RP_CH_1)")
    res = rp.rp_GenOutEnable(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenResetChannelSM(rp.RP_CH_1)")
    res = rp.rp_GenResetChannelSM(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenOutEnableSync(True)")
    res = rp.rp_GenOutEnableSync(True)
    print(res)

    print("rp.rp_GenOutDisable(rp.RP_CH_1)")
    res = rp.rp_GenOutDisable(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenOutIsEnabled(rp.RP_CH_1)")
    res = rp.rp_GenOutIsEnabled(rp.RP_CH_1)
    print(res)

def test_generator_parameters():
    print("rp.rp_GenSetAmplitudeAndOffsetOrigin(rp.RP_CH_1)")
    res = rp.rp_GenSetAmplitudeAndOffsetOrigin(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenAmp(rp.RP_CH_1,0.5)")
    res = rp.rp_GenAmp(rp.RP_CH_1,0.5)
    print(res)

    print("rp.rp_GenGetAmp(rp.RP_CH_1)")
    res = rp.rp_GenGetAmp(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenOffset(rp.RP_CH_1,0.5)")
    res = rp.rp_GenOffset(rp.RP_CH_1,0.5)
    print(res)

    print("rp.rp_GenGetOffset(rp.RP_CH_1)")
    res = rp.rp_GenGetOffset(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenFreq(rp.RP_CH_1,123456)")
    res = rp.rp_GenFreq(rp.RP_CH_1,123456)
    print(res)

    print("rp.rp_GenFreqDirect(rp.RP_CH_1,123456)")
    res = rp.rp_GenFreqDirect(rp.RP_CH_1,123456)
    print(res)

    print("rp.rp_GenGetFreq(rp.RP_CH_1)")
    res = rp.rp_GenGetFreq(rp.RP_CH_1)
    print(res)

def test_generator_sweep():
    print("rp.rp_GenSweepStartFreq(rp.RP_CH_1,123456)")
    res = rp.rp_GenSweepStartFreq(rp.RP_CH_1,123456)
    print(res)

    print("rp.rp_GenGetSweepStartFreq(rp.RP_CH_1)")
    res = rp.rp_GenGetSweepStartFreq(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenSweepEndFreq(rp.RP_CH_1,123456)")
    res = rp.rp_GenSweepEndFreq(rp.RP_CH_1,123456)
    print(res)

    print("rp.rp_GenGetSweepEndFreq(rp.RP_CH_1)")
    res = rp.rp_GenGetSweepEndFreq(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenPhase(rp.RP_CH_1,35)")
    res = rp.rp_GenPhase(rp.RP_CH_1,35)
    print(res)

    print("rp.rp_GenGetPhase(rp.RP_CH_1)")
    res = rp.rp_GenGetPhase(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenWaveform(rp.RP_CH_1,rp.RP_WAVEFORM_TRIANGLE)")
    res = rp.rp_GenWaveform(rp.RP_CH_1,rp.RP_WAVEFORM_TRIANGLE)
    print(res)

    print("rp.rp_GenGetWaveform(rp.RP_CH_1)")
    res = rp.rp_GenGetWaveform(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenSweepMode(rp.RP_CH_1,rp.RP_GEN_SWEEP_MODE_LINEAR)")
    res = rp.rp_GenSweepMode(rp.RP_CH_1,rp.RP_GEN_SWEEP_MODE_LINEAR)
    print(res)

    print("rp.rp_GenGetSweepMode(rp.RP_CH_1)")
    res = rp.rp_GenGetSweepMode(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenSweepDir(rp.RP_CH_1,rp.RP_GEN_SWEEP_DIR_UP_DOWN)")
    res = rp.rp_GenSweepDir(rp.RP_CH_1,rp.RP_GEN_SWEEP_DIR_UP_DOWN)
    print(res)

    print("rp.rp_GenGetSweepDir(rp.RP_CH_1)")
    res = rp.rp_GenGetSweepDir(rp.RP_CH_1)
    print(res)

def test_generator_arbitrary():
    buff = rp.arbBuffer(1024)
    buff2 = rp.arbBuffer(1024)
    for n in range(0,1023,1):
        buff[n] = n / 1024.0

    print("rp.rp_GenArbWaveform(rp.RP_CH_1,buff.cast(),1024)")
    res = rp.rp_GenArbWaveform(rp.RP_CH_1,buff.cast(),1024)
    print(res)

    print("rp.rp_GenGetArbWaveform(rp.RP_CH_1,buff2.cast(),1024)")
    res = rp.rp_GenGetArbWaveform(rp.RP_CH_1,buff2.cast(),1024)
    print(res,buff2[0],buff2[1],buff2[2],buff2[3],buff2[4],buff2[5])

def test_generator_numpy():
    print("Testing numpy")
    N = 1024*16
    arr_f = np.zeros(N, dtype=np.float32)

    print("rp.rp_GenGetArbWaveformNP(rp.RP_CH_1,arr_f)")
    res = rp.rp_GenGetArbWaveformNP(rp.RP_CH_1,arr_f)
    print(res,arr_f)

    print("rp.rp_GenArbWaveformNP(rp.RP_CH_1,arr_f)")
    res = rp.rp_GenArbWaveformNP(rp.RP_CH_1,arr_f)
    print(res)

    print("End testing numpy")

def test_generator_modes():
    print("rp.rp_GenDutyCycle(rp.RP_CH_1,0.2)")
    res = rp.rp_GenDutyCycle(rp.RP_CH_1,0.2)
    print(res)

    print("rp.rp_GenGetDutyCycle(rp.RP_CH_1)")
    res = rp.rp_GenGetDutyCycle(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenRiseTime(rp.RP_CH_1,0.2)")
    res = rp.rp_GenRiseTime(rp.RP_CH_1,0.2)
    print(res)

    print("rp.rp_GenGetRiseTime(rp.RP_CH_1)")
    res = rp.rp_GenGetRiseTime(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenFallTime(rp.RP_CH_1,0.2)")
    res = rp.rp_GenFallTime(rp.RP_CH_1,0.2)
    print(res)

    print("rp.rp_GenGetFallTime(rp.RP_CH_1)")
    res = rp.rp_GenGetFallTime(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenMode(rp.RP_CH_1,rp.RP_GEN_MODE_BURST)")
    res = rp.rp_GenMode(rp.RP_CH_1,rp.RP_GEN_MODE_BURST)
    print(res)

    print("rp.rp_GenGetMode(rp.RP_CH_1)")
    res = rp.rp_GenGetMode(rp.RP_CH_1)
    print(res)

def test_generator_burst():
    print("rp.rp_GenSetLoadMode(rp.RP_CH_1,rp.RP_GEN_50Ohm)")
    res = rp.rp_GenSetLoadMode(rp.RP_CH_1,rp.RP_GEN_50Ohm)
    print(res)

    print("rp.rp_GenGetLoadMode(rp.RP_CH_1)")
    res = rp.rp_GenGetLoadMode(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenBurstCount(rp.RP_CH_1,2)")
    res = rp.rp_GenBurstCount(rp.RP_CH_1,2)
    print(res)

    print("rp.rp_GenGetBurstCount(rp.RP_CH_1)")
    res = rp.rp_GenGetBurstCount(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenSetUseLastSample(rp.RP_CH_1,True)")
    res = rp.rp_GenSetUseLastSample(rp.RP_CH_1,True)
    print(res)

    print("rp.rp_GenGetUseLastSample(rp.RP_CH_1)")
    res = rp.rp_GenGetUseLastSample(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenBurstLastValue(rp.RP_CH_1,0.4)")
    res = rp.rp_GenBurstLastValue(rp.RP_CH_1,0.4)
    print(res)

    print("rp.rp_GenGetBurstLastValue(rp.RP_CH_1)")
    res = rp.rp_GenGetBurstLastValue(rp.RP_CH_1)
    print(res)

def test_generator_init_values():
    print("rp.rp_GenSetInitGenValue(rp.RP_CH_1,0.4)")
    res = rp.rp_GenSetInitGenValue(rp.RP_CH_1,0.4)
    print(res)

    print("rp.rp_GenGetInitGenValue(rp.RP_CH_1)")
    res = rp.rp_GenGetInitGenValue(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenBurstRepetitions(rp.RP_CH_1,3)")
    res = rp.rp_GenBurstRepetitions(rp.RP_CH_1,3)
    print(res)

    print("rp.rp_GenGetBurstRepetitions(rp.RP_CH_1)")
    res = rp.rp_GenGetBurstRepetitions(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenBurstPeriod(rp.RP_CH_1,1000000)")
    res = rp.rp_GenBurstPeriod(rp.RP_CH_1,1000000)
    print(res)

    print("rp.rp_GenGetBurstPeriod(rp.RP_CH_1)")
    res = rp.rp_GenGetBurstPeriod(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenBurstPeriodD(rp.RP_CH_1,1000000)")
    res = rp.rp_GenBurstPeriodD(rp.RP_CH_1,1000000)
    print(res)

    print("rp.rp_GenGetBurstPeriodD(rp.RP_CH_1)")
    res = rp.rp_GenGetBurstPeriodD(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenBurstPeriodTicks(rp.RP_CH_1,1000000)")
    res = rp.rp_GenBurstPeriodTicks(rp.RP_CH_1,1000000)
    print(res)

    print("rp.rp_GenGetBurstPeriodTicks(rp.RP_CH_1)")
    res = rp.rp_GenGetBurstPeriodTicks(rp.RP_CH_1)
    print(res)

def test_generator_triggers():
    print("rp.rp_GenTriggerSource(rp.RP_CH_1,rp.RP_GEN_TRIG_SRC_INTERNAL)")
    res = rp.rp_GenTriggerSource(rp.RP_CH_1,rp.RP_GEN_TRIG_SRC_INTERNAL)
    print(res)

    print("rp.rp_GenGetTriggerSource(rp.RP_CH_1)")
    res = rp.rp_GenGetTriggerSource(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenSynchronise()")
    res = rp.rp_GenSynchronise()
    print(res)

    print("rp.rp_GenResetTrigger(rp.RP_CH_1)")
    res = rp.rp_GenResetTrigger(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenTriggerOnly(rp.RP_CH_1)")
    res = rp.rp_GenTriggerOnly(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenTriggerOnlyBoth()")
    res = rp.rp_GenTriggerOnlyBoth()
    print(res)

def test_generator_protection():
    print("rp.rp_SetEnableTempProtection(rp.RP_CH_1,True)")
    res = rp.rp_SetEnableTempProtection(rp.RP_CH_1,True)
    print(res)

    print("rp.rp_GetEnableTempProtection(rp.RP_CH_1)")
    res = rp.rp_GetEnableTempProtection(rp.RP_CH_1)
    print(res)

    print("rp.rp_SetLatchTempAlarm(rp.RP_CH_1,True)")
    res = rp.rp_SetLatchTempAlarm(rp.RP_CH_1,True)
    print(res)

    print("rp.rp_GetLatchTempAlarm(rp.RP_CH_1)")
    res = rp.rp_GetLatchTempAlarm(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenSetGainOut(rp.RP_CH_1,rp.RP_GAIN_1X)")
    res = rp.rp_GenSetGainOut(rp.RP_CH_1,rp.RP_GAIN_1X)
    print(res)

    print("rp.rp_GenGetGainOut(rp.RP_CH_1)")
    res = rp.rp_GenGetGainOut(rp.RP_CH_1)
    print(res)

    print("rp.rp_GenSetExtTriggerDebouncerUs(123)")
    res = rp.rp_GenSetExtTriggerDebouncerUs(123)
    print(res)

    print("rp.rp_GenGetExtTriggerDebouncerUs()")
    res = rp.rp_GenGetExtTriggerDebouncerUs()
    print(res)

def test_runtime_temp():
    print("Testing runtime temperature functions")

    print("rp.rp_GetRuntimeTempAlarm(rp.RP_CH_1)")
    res = rp.rp_GetRuntimeTempAlarm(rp.RP_CH_1)
    print(res)

if __name__ == "__main__":
    init_rp()
    test_generator_basic()
    test_generator_parameters()
    test_generator_sweep()
    test_generator_arbitrary()
    test_generator_numpy()
    test_generator_modes()
    test_generator_burst()
    test_generator_init_values()
    test_generator_triggers()
    test_generator_protection()
    test_runtime_temp()