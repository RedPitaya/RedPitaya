#!/usr/bin/python3

import streaming

class Callback(streaming.ADCCallback):
    counter = {}
    fpgaLost = {}

    def receivePack(self,client,pack):
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


class ConfigCallbackImpl(streaming.ConfigCallback):

    def configConnected(self, client, host: str):
        print(f"Config client connected to {host}")

    def configError(self, client, host: str, code: int):
        print(f"Config client error on {host} code {code}")

    def configErrorTimeout(self, client, host: str):
        print(f"Config client timeout on {host}")

    def configErrorFileMissed(self, client, host: str):
        print(f"Config client error on {host}: File missed")

    def configMemoryBlockSize(self, client, host: str, block_size: int):
        print(f"Memory block size configured on {host}: {block_size} bytes")

    def configActiveChannels(self, client, host: str, channels: int):
        print(f"Active channels configured on {host}: {channels}")

    def configSuccessSend(self, client, host: str):
        print(f"Configuration sent successfully to {host}")

    def configFailSend(self, client, host: str):
        print(f"Failed to send configuration to {host}")

    def configSuccessSave(self, client, host: str):
        print(f"Configuration saved successfully on {host}")

    def configFailSave(self, client, host: str):
        print(f"Failed to save configuration on {host}")

    def configGetNewSettings(self, client, host: str):
        print(f"Getting new settings from {host}")

    def adcServerStopped(self, client, host: str):
        print(f"ADC server stopped on {host}")

    def adcServerStoppedNoActiveChannels(self, client, host: str):
        print(f"ADC server stopped on {host}: No active channels")

    def adcServerStoppedMemError(self, client, host: str):
        print(f"ADC server stopped on {host}: Memory error")

    def adcServerStoppedMemModify(self, client, host: str):
        print(f"ADC server stopped on {host}: Memory modified")

    def adcServerStoppedSDFull(self, client, host: str):
        print(f"ADC server stopped on {host}: SD card is full")

    def adcServerStoppedSDDone(self, client, host: str):
        print(f"ADC server stopped on {host}: Data written to SD card")

    def adcServerStartedTCP(self, client, host: str):
        print(f"ADC server started on {host} (TCP mode)")

    def adcServerStartedSD(self, client, host: str):
        print(f"ADC server started on {host} (SD card mode)")

    def adcServerStartedFPGA(self, client, host: str):
        print(f"ADC server started on {host} (FPGA)")

    def sigInt(self):
        obj.notifyStop()

# Master/Slave hosts
hosts = ['200.0.0.15','200.0.0.17']

# Creating a streaming client
confObj = streaming.ConfigStreamClient()
obj = streaming.ADCStreamClient(confObj)

# Creating a callback handler. And also remove the owner, since the client itself will delete the handler.
confCallback = ConfigCallbackImpl()
confObj.addCallback(confCallback)

callback = Callback()
obj.setCallback(callback)

# Enable client logs. They are disabled by default.
obj.setVerbose(True)
confObj.setVerbose(True)

# Connect to the server. Do not specify the address. If there is only one server in the network, the client will find it itself.
print("Try connect to:",hosts)
if (confObj.connect(hosts) == False):
    print("The client did not connect")
    exit(1)

# Setting up the master board

# Setting up network mode
confObj.sendConfig(hosts[0],'adc_pass_mode','NET')

# Setting up a new decimation setting
confObj.sendConfig(hosts[0],'adc_decimation','64')

# Setting the memory block size
confObj.sendConfig(hosts[0],'block_size','16384')

# Setting the size of reserved memory for ADC streaming
confObj.sendConfig(hosts[0],'adc_size','1638400')

# Turn on the first and second channels
confObj.sendConfig(hosts[0],'channel_state_1','ON')
confObj.sendConfig(hosts[0],'channel_state_2','ON')

# Receive all settings from the master server
full_config = confObj.getFileConfig(hosts[0])

# Set the same settings for the slave server
confObj.sendFileConfig(hosts[1],full_config)

if (obj.startStreaming()):
    print("Streaming is launched")
else:
    print("Error starting streaming")
    exit(1)

# Waiting for the streaming client to complete its work
obj.wait()

print("Received samples", callback.counter)
print("Number of lost samples", callback.fpgaLost)
