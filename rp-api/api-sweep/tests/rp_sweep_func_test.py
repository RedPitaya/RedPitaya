#!/usr/bin/python3


import rp_sweep
import rp

print("rp_sweep.rp_SWInit()")
print(rp_sweep.rp_SWInit())

print("rp_sweep.rp_SWRun()")
print(rp_sweep.rp_SWRun())

print("rp_sweep.rp_SWPause(False)")
print(rp_sweep.rp_SWPause(False))

print("rp_sweep.rp_SWSetStartFreq(rp.RP_CH_1,100)")
print(rp_sweep.rp_SWSetStartFreq(rp.RP_CH_1,100))

print("rp_sweep.rp_SWGetStartFreq(rp.RP_CH_1)")
print(rp_sweep.rp_SWGetStartFreq(rp.RP_CH_1))

print("rp_sweep.rp_SWSetStopFreq(rp.RP_CH_1,1000)")
print(rp_sweep.rp_SWSetStopFreq(rp.RP_CH_1,1000))

print("rp_sweep.rp_SWGetStopFreq(rp.RP_CH_1)")
print(rp_sweep.rp_SWGetStopFreq(rp.RP_CH_1))

print("rp_sweep.rp_SWSetTime(rp.RP_CH_1,200)")
print(rp_sweep.rp_SWSetTime(rp.RP_CH_1,200))

print("rp_sweep.rp_SWGetTime(rp.RP_CH_1)")
print(rp_sweep.rp_SWGetTime(rp.RP_CH_1))

print("rp_sweep.rp_SWSetMode(rp.RP_CH_1,rp.RP_GEN_SWEEP_MODE_LOG)")
print(rp_sweep.rp_SWSetMode(rp.RP_CH_1,rp.RP_GEN_SWEEP_MODE_LOG))

print("rp_sweep.rp_SWGetMode(rp.RP_CH_1)")
print(rp_sweep.rp_SWGetMode(rp.RP_CH_1))

print("rp_sweep.rp_SWSetDir(rp.RP_CH_1,rp.RP_GEN_SWEEP_DIR_UP_DOWN)")
print(rp_sweep.rp_SWSetDir(rp.RP_CH_1,rp.RP_GEN_SWEEP_DIR_UP_DOWN))

print("rp_sweep.rp_SWGetDir(rp.RP_CH_1)")
print(rp_sweep.rp_SWGetDir(rp.RP_CH_1))

print("rp_sweep.rp_SWSetDefault()")
print(rp_sweep.rp_SWSetDefault())

print("rp_sweep.rp_SWResetAll()")
print(rp_sweep.rp_SWResetAll())

print("rp_sweep.rp_SWIsAllDisabled()")
print(rp_sweep.rp_SWIsAllDisabled())

print("rp_sweep.rp_SWGenSweep(rp.RP_CH_1,True)")
print(rp_sweep.rp_SWGenSweep(rp.RP_CH_1,True))

print("rp_sweep.rp_SWIsGen(rp.RP_CH_1)")
print(rp_sweep.rp_SWIsGen(rp.RP_CH_1))

print("rp_sweep.rp_SWStop()")
print(rp_sweep.rp_SWStop())

print("rp_sweep.rp_SWRelease()")
print(rp_sweep.rp_SWRelease())
