#!/usr/bin/python3

import rp_la

class Callback(rp_la.CLACallback):
    def captureStatus(self,controller,isTimeout):
        print("captureStatus timeout =",isTimeout)


obj = rp_la.CLAController()

callback = Callback()
obj.setDelegate(callback.__disown__())

obj.setEnableRLE(True)
obj.setDecimation(1)
obj.setTrigger(0,rp_la.LA_RISING_OR_FALLING)
obj.setPreTriggerSamples(1024)
obj.setPostTriggerSamples(1024)
obj.runAsync(0)
print(obj.wait(10000))
obj.saveCaptureDataToFile("/tmp/data.bin")
del obj
