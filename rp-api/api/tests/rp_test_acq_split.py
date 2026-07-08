#!/usr/bin/python3

import rp
import time

def init_rp():
    print("rp.rp_InitAddresses()")
    res = rp.rp_InitAddresses()
    print(res)

    print("rp.rp_Init()")
    res = rp.rp_Init()
    print(res)

def test_split_trigger():
    print("Testing split trigger functions")

    print("rp.rp_AcqSetSplitTrigger(True)")
    res = rp.rp_AcqSetSplitTrigger(True)
    print(res)

    print("rp.rp_AcqGetSplitTrigger()")
    res = rp.rp_AcqGetSplitTrigger()
    print(res)

    print("rp.rp_AcqSetSplitTriggerPass(False)")
    res = rp.rp_AcqSetSplitTriggerPass(False)
    print(res)

    print("rp.rp_AcqGetSplitTriggerPass()")
    res = rp.rp_AcqGetSplitTriggerPass()
    print(res)

def test_split_trigger_channels():
    print("rp.rp_AcqSetArmKeepCh(rp.RP_CH_1, False)")
    res = rp.rp_AcqSetArmKeepCh(rp.RP_CH_1, False)
    print(res)

    print("rp.rp_AcqGetArmKeepCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetArmKeepCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetBufferFillStateCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetBufferFillStateCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqSetDecimationCh(rp.RP_CH_1, 1)")
    res = rp.rp_AcqSetDecimationCh(rp.RP_CH_1, 1)
    print(res)

    print("rp.rp_AcqGetDecimationCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetDecimationCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqSetDecimationFactorCh(rp.RP_CH_1, 1)")
    res = rp.rp_AcqSetDecimationFactorCh(rp.RP_CH_1, 1)
    print(res)

    print("rp.rp_AcqGetDecimationFactorCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetDecimationFactorCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetSamplingRateHzCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetSamplingRateHzCh(rp.RP_CH_1)
    print(res)

def test_split_trigger_sources():
    print("rp.rp_AcqSetTriggerSrcCh(rp.RP_CH_1, rp.RP_TRIG_SRC_NOW)")
    res = rp.rp_AcqSetTriggerSrcCh(rp.RP_CH_1, rp.RP_TRIG_SRC_NOW)
    print(res)

    print("rp.rp_AcqGetTriggerSrcCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetTriggerSrcCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetTriggerStateCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetTriggerStateCh(rp.RP_CH_1)
    print(res)

def test_split_trigger_delay():
    print("rp.rp_AcqSetTriggerDelayCh(rp.RP_CH_1, 1)")
    res = rp.rp_AcqSetTriggerDelayCh(rp.RP_CH_1, 1)
    print(res)

    print("rp.rp_AcqGetTriggerDelayCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetTriggerDelayCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqSetTriggerDelayDirectCh(rp.RP_CH_1, 1)")
    res = rp.rp_AcqSetTriggerDelayDirectCh(rp.RP_CH_1, 1)
    print(res)

    print("rp.rp_AcqGetTriggerDelayDirectCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetTriggerDelayDirectCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqSetTriggerDelayNsCh(rp.RP_CH_1, 10000)")
    res = rp.rp_AcqSetTriggerDelayNsCh(rp.RP_CH_1, 10000)
    print(res)

    print("rp.rp_AcqGetTriggerDelayNsCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetTriggerDelayNsCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqSetTriggerDelayNsDirectCh(rp.RP_CH_1, 10000)")
    res = rp.rp_AcqSetTriggerDelayNsDirectCh(rp.RP_CH_1, 10000)
    print(res)

    print("rp.rp_AcqGetTriggerDelayNsDirectCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetTriggerDelayNsDirectCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetPreTriggerCounterCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetPreTriggerCounterCh(rp.RP_CH_1)
    print(res)

def test_split_trigger_control():
    print("rp.rp_AcqGetWritePointerCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetWritePointerCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetWritePointerAtTrigCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetWritePointerAtTrigCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqStartCh(rp.RP_CH_1)")
    res = rp.rp_AcqStartCh(rp.RP_CH_1)
    print(res)

    time.sleep(1)

    print("rp.rp_AcqStopCh(rp.RP_CH_1)")
    res = rp.rp_AcqStopCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqResetCh(rp.RP_CH_1)")
    res = rp.rp_AcqResetCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqUnlockTriggerCh(rp.RP_CH_1)")
    res = rp.rp_AcqUnlockTriggerCh(rp.RP_CH_1)
    print(res)

    print("rp.rp_AcqGetUnlockTriggerCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetUnlockTriggerCh(rp.RP_CH_1)
    print(res)

def test_split_trigger_averaging():
    print("rp.rp_AcqSetAveragingCh(rp.RP_CH_1,True)")
    res = rp.rp_AcqSetAveragingCh(rp.RP_CH_1,True)
    print(res)

    print("rp.rp_AcqGetAveragingCh(rp.RP_CH_1)")
    res = rp.rp_AcqGetAveragingCh(rp.RP_CH_1)
    print(res)

if __name__ == "__main__":
    init_rp()
    test_split_trigger()
    test_split_trigger_channels()
    test_split_trigger_sources()
    test_split_trigger_delay()
    test_split_trigger_control()
    test_split_trigger_averaging()