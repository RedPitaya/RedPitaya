#!/usr/bin/python3


import rp


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



print("rp.rp_Release()")
res = rp.rp_Release()
print(res)