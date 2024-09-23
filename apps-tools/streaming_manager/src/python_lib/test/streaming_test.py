#!/usr/bin/python3

import time
import streaming

class Callback(streaming.ADCCallback):
    def recievePack(self,n):
        print(n.host)
        print(n.channel1.samples)
        print(n.channel1.fpgaLost)
        print(n.channel1.bitsBySample)
        print(n.channel1.packId)
#        print(n.channel1.raw)

print("streaming.ADCStreamClient()")
obj = streaming.ADCStreamClient()
print(obj)

callback = Callback()
callback.thisown = 0
obj.setReciveDataFunction(callback)

print("obj.setVerbose(True)")
obj.setVerbose(True)

print('obj.connect()')
print(obj.connect())

x = obj.getFileConfig()
print(x)
obj.sendFileConfig(x)

print("obj.getConfig('adc_decimation')")
print(obj.getConfig('adc_decimation'))

print("obj.sendConfig('adc_decimation','4')")
print(obj.sendConfig('adc_decimation','4'))


print("obj.startStreaming()")
print(obj.startStreaming())


print("obj.stopStreaming()")
print(obj.stopStreaming())

