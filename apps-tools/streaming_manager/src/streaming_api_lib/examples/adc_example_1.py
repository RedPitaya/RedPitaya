#!/usr/bin/python3

import streaming

class Callback(streaming.ADCCallback):
    counter = 0
    fpgaLost = 0

    def recievePack(self,client,n):
        self.counter = self.counter + n.channel1.samples + n.channel2.samples
        self.fpgaLost = self.fpgaLost + max([n.channel1.fpgaLost, n.channel2.fpgaLost, n.channel3.fpgaLost, n.channel4.fpgaLost])
        # Count the number of samples received and stop streaming
        if (self.counter > 5e7):
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
        print("Control client error",host, ". Connection timeout")


# Creating a streaming client
obj = streaming.ADCStreamClient()

# Creating a callback handler. And also remove the owner, since the client itself will delete the handler.
callback = Callback()
obj.setReciveDataFunction(callback.__disown__())

# Disable client logs. They are disabled by default.
obj.setVerbose(False)

# Connect to the server. Do not specify the address. If there is only one server in the network, the client will find it itself.
if (obj.connect() == False):
    print("The client did not connect")
    exit(1)

# Get the current decimation setting
current_decimation = obj.getConfig('adc_decimation');
print("Current decimation",current_decimation)


# Setting up network mode
obj.sendConfig('adc_pass_mode','NET')

# Setting up a new decimation setting
obj.sendConfig('adc_decimation','64')

# Setting the memory block size
obj.sendConfig('block_size','16384')

# Setting the size of reserved memory for ADC streaming
obj.sendConfig('adc_size','1638400')

# Turn on the first and second channels
obj.sendConfig('channel_state_1','ON')
obj.sendConfig('channel_state_2','ON')


if (obj.startStreaming()):
    print("Streaming is launched")
else:
    print("Error starting streaming")
    exit(1)

# Waiting for the streaming client to complete its work
obj.wait()

print("Received samples", callback.counter)
print("Number of lost samples", callback.fpgaLost)
