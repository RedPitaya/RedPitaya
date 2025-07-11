#!/usr/bin/python3

import streaming

class Callback(streaming.ADCCallback):
    counter = {}
    fpgaLost = {}

    def recievePack(self,client,pack):
        if pack.host in self.counter.keys():
            self.counter[pack.host] += sum([pack.channel1.samples,pack.channel2.samples])
        else:
            self.counter[pack.host] = sum([pack.channel1.samples,pack.channel2.samples])

        if pack.host in self.fpgaLost.keys():
            self.fpgaLost[pack.host] += max([pack.channel1.fpgaLost,pack.channel2.fpgaLost])
        else:
            self.fpgaLost[pack.host] = max([pack.channel1.fpgaLost,pack.channel2.fpgaLost])

        # Count the number of samples received and stop streaming
        if (self.counter[pack.host]  > 5e7):
            client.notifyStop(pack.host)

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


# Master/Slave hosts
hosts = ['200.0.0.7','200.0.0.8']

# Creating a streaming client
obj = streaming.ADCStreamClient()

# Creating a callback handler. And also remove the owner, since the client itself will delete the handler.
callback = Callback()
obj.setReciveDataFunction(callback.__disown__())

# Disable client logs. They are disabled by default.
obj.setVerbose(False)

# Connect to the server. Do not specify the address. If there is only one server in the network, the client will find it itself.
print("Try connect to:",hosts)
if (obj.connect(hosts) == False):
    print("The client did not connect")
    exit(1)

# Setting up the master board

# Setting up network mode
obj.sendConfig(hosts[0],'adc_pass_mode','NET')

# Setting up a new decimation setting
obj.sendConfig(hosts[0],'adc_decimation','64')

# Setting the memory block size
obj.sendConfig(hosts[0],'block_size','16384')

# Setting the size of reserved memory for ADC streaming
obj.sendConfig(hosts[0],'adc_size','1638400')

# Turn on the first and second channels
obj.sendConfig(hosts[0],'channel_state_1','ON')
obj.sendConfig(hosts[0],'channel_state_2','ON')

# Receive all settings from the master server
full_config = obj.getFileConfig(hosts[0])

# Set the same settings for the slave server
obj.sendFileConfig(hosts[1],full_config)

if (obj.startStreaming()):
    print("Streaming is launched")
else:
    print("Error starting streaming")
    exit(1)

# Waiting for the streaming client to complete its work
obj.wait()

print("Received samples", callback.counter)
print("Number of lost samples", callback.fpgaLost)
