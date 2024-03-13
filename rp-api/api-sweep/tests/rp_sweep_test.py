#!/usr/bin/python3


import rp_sweep
import rp

print("rp.rp_Init()")
res = rp.rp_Init()
print(res)

print("rp_sweep.CSweepController()")
obj = rp_sweep.CSweepController()
print(obj)

print("obj.run()")
res = obj.run()
print(res)

print("obj.pause(False)")
res = obj.pause(False)
print(res)

print("obj.pause(True)")
res = obj.pause(True)
print(res)

print("obj.setStartFreq(rp.RP_CH_1,100)")
res = obj.setStartFreq(rp.RP_CH_1,100)
print(res)

print("obj.getStartFreq(rp.RP_CH_1)")
res = obj.getStartFreq(rp.RP_CH_1)
print(res)

print("obj.setStopFreq(rp.RP_CH_1,10000)")
res = obj.setStopFreq(rp.RP_CH_1,10000)
print(res)

print("obj.getStopFreq(rp.RP_CH_1)")
res = obj.getStopFreq(rp.RP_CH_1)
print(res)

print("obj.setTime(rp.RP_CH_1,100000)")
res = obj.setTime(rp.RP_CH_1,100000)
print(res)

print("obj.getTime(rp.RP_CH_1)")
res = obj.getTime(rp.RP_CH_1)
print(res)

print("obj.setMode(rp.RP_CH_1,rp.RP_GEN_SWEEP_MODE_LOG)")
res = obj.setMode(rp.RP_CH_1,rp.RP_GEN_SWEEP_MODE_LOG)
print(res)

print("obj.getMode(rp.RP_CH_1)")
res = obj.getMode(rp.RP_CH_1)
print(res)

print("obj.setDir(rp.RP_CH_1,rp.RP_GEN_SWEEP_DIR_UP_DOWN)")
res = obj.setDir(rp.RP_CH_1,rp.RP_GEN_SWEEP_DIR_UP_DOWN)
print(res)

print("obj.getDir(rp.RP_CH_1)")
res = obj.getDir(rp.RP_CH_1)
print(res)

print("obj.resetAll()")
res = obj.resetAll()
print(res)

print("obj.isAllDisabled()")
res = obj.isAllDisabled()
print(res)

print("obj.genSweep(rp.RP_CH_1,True)")
res = obj.genSweep(rp.RP_CH_1,True)
print(res)

print("obj.isGen(rp.RP_CH_1)")
res = obj.isGen(rp.RP_CH_1)
print(res)

print("obj.isAllDisabled()")
res = obj.isAllDisabled()
print(res)

print("obj.stop()")
res = obj.stop()
print(res)

print("rp.rp_Release()")
res = rp.rp_Release()
print(res)
