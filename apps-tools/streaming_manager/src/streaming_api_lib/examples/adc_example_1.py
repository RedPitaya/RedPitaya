#!/usr/bin/python3

import streaming


class Callback(streaming.ADCCallback):
    counter = 0
    fpgaLost = 0

    def receivePack(self,client,n):
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


# Creating a streaming client
confObj = streaming.ConfigStreamClient()
obj = streaming.ADCStreamClient(confObj)

# Creating a callback handler. And also remove the owner, since the client itself will delete the handler.
confCallback = ConfigCallbackImpl()
confObj.addCallback(confCallback)

callback = Callback()
obj.setCallback(callback)

# Enable client logs. They are disabled by default.
confObj.setVerbose(True)
obj.setVerbose(True)

# Connect to the server. Do not specify the address. If there is only one server in the network, the client will find it itself.
if (confObj.connect() == False):
    print("The client did not connect")
    exit(1)

# Get the current decimation setting
current_decimation = confObj.getConfig('adc_decimation');
print("Current decimation",current_decimation)


# Setting up network mode
confObj.sendConfig('adc_pass_mode','NET')

# Setting up a new decimation setting
confObj.sendConfig('adc_decimation','64')

# Setting the memory block size
confObj.sendConfig('block_size','16384')

# Setting the size of reserved memory for ADC streaming
confObj.sendConfig('adc_size','1638400')

# Turn on the first and second channels
confObj.sendConfig('channel_state_1','ON')
confObj.sendConfig('channel_state_2','ON')


if (obj.startStreaming()):
    print("Streaming is launched")
else:
    print("Error starting streaming")
    exit(1)

# Waiting for the streaming client to complete its work
obj.wait()

print("Received samples", callback.counter)
print("Number of lost samples", callback.fpgaLost)
