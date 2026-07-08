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

    print("rp.rp_InitReset(True)")
    res = rp.rp_InitReset(True)
    print(res)

    print("rp.rp_IsApiInit()")
    res = rp.rp_IsApiInit()
    print(res)

def test_basic_functions():
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

    print("rp.rp_Release()")
    res = rp.rp_Release()
    print(res)

def test_led_gpio():
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

    print("rp.rp_DpinReset()")
    res = rp.rp_DpinReset()
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

def test_freq_sync():
    print("rp.rp_GetFreqCounter()")
    res = rp.rp_GetFreqCounter()
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

    print("rp.rp_GetCANModeEnable()")
    res = rp.rp_GetCANModeEnable()
    print(res)

    print("rp.rp_SetCANModeEnable(False)")
    res = rp.rp_SetCANModeEnable(False)
    print(res)

    print("rp.rp_GetSourceTrigOutput()")
    res = rp.rp_GetSourceTrigOutput()
    print(res)

    print("rp.rp_SetSourceTrigOutput(rp.OUT_TR_DAC)")
    res = rp.rp_SetSourceTrigOutput(rp.OUT_TR_DAC)
    print(res)

    print("rp.rp_GetEnableDaisyChainClockSync()")
    res = rp.rp_GetEnableDaisyChainClockSync()
    print(res)

    print("rp.rp_SetEnableDaisyChainClockSync(False)")
    res = rp.rp_SetEnableDaisyChainClockSync(False)
    print(res)

    print("rp.rp_EnableDebugReg()")
    res = rp.rp_EnableDebugReg()
    print(res)

def test_registers():
    print("rp.rp_PrintHouseRegset()")
    res = rp.rp_PrintHouseRegset()
    print(res)

    print("rp.rp_PrintOscRegset()")
    res = rp.rp_PrintOscRegset()
    print(res)

    print("rp.rp_PrintAsgRegset()")
    res = rp.rp_PrintAsgRegset()
    print(res)

    print("rp.rp_PrintAmsRegset()")
    res = rp.rp_PrintAmsRegset()
    print(res)

    print("rp.rp_PrintDaisyRegset()")
    res = rp.rp_PrintDaisyRegset()
    print(res)

def test_pll_triggers():
    print("rp.rp_GetPllControlEnable()")
    res = rp.rp_GetPllControlEnable()
    print(res)

    print("rp.rp_SetPllControlEnable(False)")
    res = rp.rp_SetPllControlEnable(False)
    print(res)

    print("rp.rp_GetPllControlLocked()")
    res = rp.rp_GetPllControlLocked()
    print(res)

    print("rp.rp_SetExternalTriggerLevel(1)")
    res = rp.rp_SetExternalTriggerLevel(1)
    print(res)

    print("rp.rp_GetExternalTriggerLevel()")
    res = rp.rp_GetExternalTriggerLevel()
    print(res)

if __name__ == "__main__":
    init_rp()
    test_basic_functions()
    test_led_gpio()
    test_freq_sync()
    test_registers()
    test_pll_triggers()