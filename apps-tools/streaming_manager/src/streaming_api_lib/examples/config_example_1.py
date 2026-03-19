#!/usr/bin/python3

import streaming

class Callback(streaming.ConfigCallback):

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

    def dacServerStartedTCP(self, client, host: str):
        print(f"DAC server started on {host} (TCP mode)")

    def dacServerStartedSD(self, client, host: str):
        print(f"DAC server started on {host} (SD card mode)")

    def dacServerStoppedMemError(self, client, host: str):
        print(f"DAC server stopped on {host}: Memory error")

    def dacServerStoppedMemModify(self, client, host: str):
        print(f"DAC server stopped on {host}: Memory modified")

    def dacServerStoppedConfigError(self, client, host: str):
        print(f"DAC server stopped on {host}: Configuration error")

    def dacServerStoppedFileMissed(self, client, host: str):
        print(f"DAC server stopped on {host}: File missed")

    def dacServerStoppedSDDone(self, client, host: str):
        print(f"DAC server stopped on {host}: SD card operation completed")

    def dacServerStoppedSDEmpty(self, client, host: str):
        print(f"DAC server stopped on {host}: SD card is empty")

    def dacServerStoppedSDBroken(self, client, host: str):
        print(f"DAC server stopped on {host}: SD card is broken")

    def dacServerStoppedSDMissing(self, client, host: str):
        print(f"DAC server stopped on {host}: SD card is missing")


# Creating a streaming client
obj = streaming.ConfigStreamClient()

# Creating a callback handler. And also remove the owner, since the client itself will delete the handler.
callback = Callback()
obj.addCallback(callback)

# Enable client logs. They are disabled by default.
obj.setVerbose(True)

# Connect to the server. Do not specify the address. If there is only one server in the network, the client will find it itself.
if (obj.connect() == False):
    print("The client did not connect")
    exit(1)

# Get the current decimation setting
current_decimation = obj.getConfig('adc_decimation');
print("Current decimation",current_decimation)

first_host = obj.getHosts()[0]

obj.requestMemoryBlockSize(first_host)
obj.requestActiveChannels(first_host)

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