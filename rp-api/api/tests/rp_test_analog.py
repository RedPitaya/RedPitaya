#!/usr/bin/python3

import rp

def init_rp():
    print("rp.rp_InitAdressess()")
    res = rp.rp_InitAdressess()
    print(res)

    print("rp.rp_Init()")
    res = rp.rp_Init()
    print(res)

def test_analog_pins():
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

def test_analog_outputs():
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

if __name__ == "__main__":
    init_rp()
    test_analog_pins()
    test_analog_outputs()