#!/usr/bin/python3

from time import sleep
import streaming

class Callback(streaming.ADCCallback):
    counter = 0

    def recievePack(self,client,n):
        self.counter = self.counter + 1
#        print(n.host)
#        print(n.channel1.samples)
#        print(n.channel1.fpgaLost)
#        print(n.channel1.bitsBySample)
#        print(n.channel1.packId)
#        print(n.channel1.raw)
        if (self.counter > 10000):
            client.notifyStop()

    def connected(self,client,host):
        print("Client connected",host)

    def disconnected(self,client,host):
        print("Client disconnected",host)

    def error(self,client,host,code):
        print("Client error",host, "code" , code)

    def stopped(self,client,host,code):
        print("Server stopped",host)

    def stoppedNoActiveChannels(self,client,host):
        print("Server stopped",host,". No active channels.")

    def stoppedMemError(self,client,host):
        print("Server stopped",host,". Memory error.")

    def stoppedMemModify(self,client,host):
        print("Server stopped",host,". Memory changed")

    def stoppedSDFull(self,client,host):
        print("Server stopped",host,". SD is full")

    def stoppedSDDone(self,client,host):
        print("Server stopped",host,". The data is written to the memory card.")

    def configConnected(self,client,host):
        print("Control client connected",host)

    def configError(self,client,host,code):
        print("Control client error",host, "code" , code)

    def configErrorTimeout(self,client,host):
        print("Control client error",host, ". Сonnection timeout")


print("streaming.ADCStreamClient()")
obj = streaming.ADCStreamClient()
print(obj)

callback = Callback()
callback.thisown = 0
obj.setReciveDataFunction(callback)

print("obj.setVerbose(True)")
#obj.setVerbose(True)

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

print("obj.wait()")
print(obj.wait())

print("obj.stopStreaming()")
print(obj.stopStreaming())

print("Received packages ", callback.counter)
