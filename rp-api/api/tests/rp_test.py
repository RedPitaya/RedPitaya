#!/usr/bin/python3


import rp

# rp.h

print("rp.rp_Init()")
res = rp.rp_Init()
print(res)


print("rp.rp_InitReset(True)")
res = rp.rp_InitReset(True)
print(res)

print("rp.rp_IsApiInit()")
res = rp.rp_IsApiInit()
print(res)

print("rp.rp_GetVersion()")
res = rp.rp_GetVersion()
print(res)

print("rp.rp_Reset()")
res = rp.rp_Reset()
print(res)

print("rp.rp_GetError(rp.RP_EOED)")
res = rp.rp_GetError(rp.RP_EOED)
print(res)

print("rp.rp_EnableDigitalLoop(False)")
res = rp.rp_EnableDigitalLoop(False)
print(res)

print("rp.rp_IdGetID()")
res = rp.rp_IdGetID()
print(res)

print("rp.rp_IdGetDNA()")
res = rp.rp_IdGetDNA()
print(res)


print("rp.rp_LEDGetState()")
res = rp.rp_LEDGetState()
print(res)


print("rp.rp_LEDSetState(0)")
res = rp.rp_LEDSetState(0)
print(res)


print("rp.rp_GPIOnGetDirection()")
res = rp.rp_GPIOnGetDirection()
print(res)

print("rp.rp_GPIOnSetDirection(0)")
res = rp.rp_GPIOnSetDirection(0)
print(res)

print("rp.rp_GPIOnGetState()")
res = rp.rp_GPIOnGetState()
print(res)

print("rp.rp_GPIOnSetState(0)")
res = rp.rp_GPIOnSetState(0)
print(res)

print("rp.rp_GPIOpGetDirection()")
res = rp.rp_GPIOpGetDirection()
print(res)

print("rp.rp_GPIOpSetDirection(0)")
res = rp.rp_GPIOpSetDirection(0)
print(res)

print("rp.rp_GPIOpGetState()")
res = rp.rp_GPIOpGetState()
print(res)

print("rp.rp_GPIOpSetState(0)")
res = rp.rp_GPIOpSetState(0)
print(res)

print("rp.rp_DpinGetDirection(rp.RP_DIO0_P)")
res = rp.rp_DpinGetDirection(rp.RP_DIO0_P)
print(res)

print("rp.rp_DpinSetDirection(rp.RP_DIO0_P,rp.RP_IN)")
res = rp.rp_DpinSetDirection(rp.RP_DIO0_P,rp.RP_IN)
print(res)

print("rp.rp_DpinGetState(rp.RP_DIO0_P)")
res = rp.rp_DpinGetState(rp.RP_DIO0_P)
print(res)

print("rp.rp_DpinSetState(rp.RP_DIO0_P,rp.RP_HIGH)")
res = rp.rp_DpinSetState(rp.RP_DIO0_P,rp.RP_HIGH)
print(res)

print("rp.rp_GetEnableDaisyChainTrigSync()")
res = rp.rp_GetEnableDaisyChainTrigSync()
print(res)

print("rp.rp_SetEnableDaisyChainTrigSync(False)")
res = rp.rp_SetEnableDaisyChainTrigSync(False)
print(res)

print("rp.rp_GetDpinEnableTrigOutput()")
res = rp.rp_GetDpinEnableTrigOutput()
print(res)

print("rp.rp_SetDpinEnableTrigOutput(False)")
res = rp.rp_SetDpinEnableTrigOutput(False)
print(res)

print("rp.rp_GetSourceTrigOutput()")
res = rp.rp_GetSourceTrigOutput()
print(res)

print("rp.rp_SetSourceTrigOutput(rp.OUT_TR_DAC)")
res = rp.rp_SetSourceTrigOutput(rp.OUT_TR_DAC)
print(res)

print("rp.rp_GetEnableDiasyChainClockSync()")
res = rp.rp_GetEnableDiasyChainClockSync()
print(res)

print("rp.rp_SetEnableDiasyChainClockSync(False)")
res = rp.rp_SetEnableDiasyChainClockSync(False)
print(res)

print("rp.rp_EnableDebugReg()")
res = rp.rp_EnableDebugReg()
print(res)



print("rp.rp_ApinReset()")
res = rp.rp_ApinReset()
print(res)

print("rp.rp_ApinGetValue(rp.RP_AIN0)")
res = rp.rp_ApinGetValue(rp.RP_AIN0)
print(res)

print("rp.rp_ApinGetValueRaw(rp.RP_AIN0)")
res = rp.rp_ApinGetValueRaw(rp.RP_AIN0)
print(res)

print("rp.rp_ApinSetValue(rp.RP_AOUT0,1)")
res = rp.rp_ApinSetValue(rp.RP_AOUT0,1)
print(res)

print("rp.rp_ApinSetValueRaw(rp.RP_AOUT0,0x50)")
res = rp.rp_ApinSetValueRaw(rp.RP_AOUT0,0x50)
print(res)

print("rp.rp_ApinGetRange(rp.RP_AIN0)")
res = rp.rp_ApinGetRange(rp.RP_AIN0)
print(res)

print("rp.rp_AIpinGetValue(0)")
res = rp.rp_AIpinGetValue(0)
print(res)

print("rp.rp_AIpinGetValueRaw(0)")
res = rp.rp_AIpinGetValueRaw(0)
print(res)


print("rp.rp_AOpinReset()")
res = rp.rp_ApinReset()
print(res)

print("rp.rp_AOpinGetValue(0)")
res = rp.rp_AOpinGetValue(0)
print(res)

print("rp.rp_AOpinGetValueRaw(0)")
res = rp.rp_AOpinGetValueRaw(0)
print(res)

print("rp.rp_AOpinSetValue(0,1)")
res = rp.rp_AOpinSetValue(0,1)
print(res)

print("rp.rp_AOpinSetValueRaw(0,0x50)")
res = rp.rp_AOpinSetValueRaw(0,0x50)
print(res)

print("rp.rp_AOpinGetRange(0)")
res = rp.rp_AOpinGetRange(0)
print(res)


print("rp.rp_GetPllControlEnable()")
res = rp.rp_GetPllControlEnable()
print(res)

print("rp.rp_SetPllControlEnable(False)")
res = rp.rp_SetPllControlEnable(False)
print(res)

print("rp.rp_GetPllControlLocked()")
res = rp.rp_GetPllControlLocked()
print(res)


# rp_gen.h

print("rp.rp_GenReset()")
res = rp.rp_GenReset()
print(res)

print("rp.rp_GenOutEnable(rp.RP_CH_1)")
res = rp.rp_GenOutEnable(rp.RP_CH_1)
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
res = rp.rp_GenOffset(rp.RP_CH_1,123456)
print(res)

print("rp.rp_GenFreqDirect(rp.RP_CH_1,123456)")
res = rp.rp_GenFreqDirect(rp.RP_CH_1,123456)
print(res)

print("rp.rp_GenGetFreq(rp.RP_CH_1)")
res = rp.rp_GenGetOffset(rp.RP_CH_1)
print(res)

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

buff = rp.arbBuffer(1024)
buff2 = rp.arbBuffer(1024)
for n in range(0,1023,1):
    buff[n] = n / 1024.0

print("rp.rp_GenArbWaveform(rp.RP_CH_1,buff,1024)")
res = rp.rp_GenArbWaveform(rp.RP_CH_1,buff,1024)
print(res)

print("rp.rp_GenGetArbWaveform(rp.RP_CH_1,buff2)")
res = rp.rp_GenGetArbWaveform(rp.RP_CH_1,buff2)
print(res,buff2[0],buff2[1],buff2[2])


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


print("rp.rp_GenBurstCount(rp.RP_CH_1,rp.RP_GEN_MODE_BURST)")
res = rp.rp_GenBurstCount(rp.RP_CH_1,rp.RP_GEN_MODE_BURST)
print(res)

print("rp.rp_GenGetBurstCount(rp.RP_CH_1)")
res = rp.rp_GenGetBurstCount(rp.RP_CH_1)
print(res)

print("rp.rp_GenBurstCount(rp.RP_CH_1,2)")
res = rp.rp_GenBurstCount(rp.RP_CH_1,2)
print(res)

print("rp.rp_GenGetBurstCount(rp.RP_CH_1)")
res = rp.rp_GenGetBurstCount(rp.RP_CH_1)
print(res)


print("rp.rp_GenBurstLastValue(rp.RP_CH_1,0.4)")
res = rp.rp_GenBurstLastValue(rp.RP_CH_1,0.4)
print(res)

print("rp.rp_GenGetBurstLastValue(rp.RP_CH_1)")
res = rp.rp_GenGetBurstLastValue(rp.RP_CH_1)
print(res)

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

print("rp.rp_Release()")
res = rp.rp_Release()
print(res)