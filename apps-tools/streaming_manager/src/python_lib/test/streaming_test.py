#!/usr/bin/python3

import time
import streaming

class Callback(streaming.ADCCallback):
    def recievePack(self,n):
        print(n.host)
        print(n.fpgaLost)
        print(n.ch1_raw)
        print(n.ch2_raw)


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

print("obj.sendConfig('adc_decimation','4')")
print(obj.sendConfig('adc_decimation','4'))

print("obj.getConfig('adc_decimation')")
print(obj.getConfig('adc_decimation'))

print("obj.getConfig('127.0.0.01','adc_decimation')")
print(obj.getConfig('127.0.0.01','adc_decimation'))

print("obj.startStreaming()")
print(obj.startStreaming())

time.sleep(3)

print("obj.stopStreaming()")
print(obj.stopStreaming())