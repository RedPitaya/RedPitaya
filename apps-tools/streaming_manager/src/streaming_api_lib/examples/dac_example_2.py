#!/usr/bin/python3

import streaming
import numpy as np
import wave
import struct

class Callback(streaming.DACCallback):
    counter = 0

    def sendedPack(self,client, ch1_size, ch2_size):
        #print("Data sent CH1",ch1_size,"CH2",ch2_size)
        self.counter = self.counter + 1

    def connected(self,client,host):
        print("DAC client connected",host)

    def disconnected(self,client,host):
        print("DAC client disconnected",host)

    def error(self,client,host,code):
        print("DAC client error", host, "code" , code)
        client.notifyStop()

    def stopped(self,client,host):
        print("DAC server stopped",host)
        client.notifyStop()

    def stoppedFileEnd(self,client,host):
        print("DAC server stopped. The file is finished",host)
        client.notifyStop()

    def stoppedFileBroken(self,client,host):
        print("DAC server stopped. The file is broken",host)
        client.notifyStop()

    def stoppedEmpty(self,client,host):
        print("DAC server stopped. The file is empty",host)
        client.notifyStop()

    def stoppedMemError(self,client,host):
        print("DAC server stopped",host,". Memory error.")
        client.notifyStop()

    def stoppedMemModify(self,client,host):
        print("DAC server stopped",host,". Memory changed")
        client.notifyStop()

    def stoppedMissingFile(self,client,host):
        print("DAC server stopped",host,". Missing file")
        client.notifyStop()

    def configConnected(self,client,host):
        print("Control client connected",host)
        client.notifyStop()

    def configError(self,client,host,code):
        print("Control client error",host, "code" , code)
        client.notifyStop()

    def configErrorTimeout(self,client,host):
        print("Control client error",host, ". Connection timeout")
        client.notifyStop()

def create_stereo_wav(filename, left_channel, right_channel, sample_rate):
    """Create a stereo WAV file using Python's built-in wave module"""
    with wave.open(filename, 'w') as wav_file:
        # Set parameters: 2 channels, 1 byte per sample (8-bit), sample rate
        wav_file.setnchannels(2)  # stereo
        wav_file.setsampwidth(1)  # 1 byte per sample (8-bit)
        wav_file.setframerate(sample_rate)

        # Interleave left and right channels
        stereo_data = np.empty((left_channel.size + right_channel.size,), dtype=np.uint8)
        stereo_data[0::2] = left_channel
        stereo_data[1::2] = right_channel

        # Convert to bytes and write
        wav_file.writeframes(stereo_data.tobytes())

# Creating a streaming client
obj = streaming.DACStreamClient()

# Creating a callback handler. And also remove the owner, since the client itself will delete the handler.
callback = Callback()
obj.setCallbackFunction(callback.__disown__())

# Disable client logs. They are disabled by default.
obj.setVerbose(True)

# Connect to the server. Do not specify the address. If there is only one server in the network, the client will find it itself.
if (obj.connect() == False):
    print("The client did not connect")
    exit(1)

# Get the current decimation setting
dac_rate = obj.getConfig('dac_rate');
print("Current rate",dac_rate)


# Setting up network mode
obj.sendConfig('dac_pass_mode','DAC_NET')

# Setting up a new decimation setting
obj.sendConfig('dac_rate','125000000')

# Setting the memory block size
obj.sendConfig('block_size','262144')

# Setting the size of reserved memory for DAC streaming
obj.sendConfig('adc_size','2621440')

# Generate sine wave data (8-bit)
frequency = 1
sampling_rate = 1024 * 256
t = np.linspace(0., 1., sampling_rate)
amplitude = np.iinfo(np.int8).max
data = (amplitude * np.sin(2. * np.pi * frequency * t)).astype(np.int8)

# Create stereo data (same signal on both channels)
stereo_left = data
stereo_right = data

# Create WAV file using wave module
create_stereo_wav("sin.wav", stereo_left, stereo_right, sampling_rate)

obj.setRepeatInf(True)

if (obj.startStreamingWAV("./sin.wav")):
    print("Streaming is launched")
else:
    print("Error starting streaming")
    exit(1)

# Waiting for the streaming client to complete its work
obj.wait()

print("Send pack", callback.counter)
