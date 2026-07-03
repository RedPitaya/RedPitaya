#!/usr/bin/python3

import streaming
import math
import numpy as np

class Callback(streaming.DACCallback):
    counter = 0


    def streamData8Bit(self, client, ch1, ch2, size):
        try:
            # 1. Create a time array from 0 to 2*PI
            t = np.linspace(0, 2 * np.pi, size, endpoint=False, dtype=np.float32)

            # 2. Calculate sine and scale to int8 (-127 to 127)
            # We use .astype(np.int8) to ensure the data format matches C++
            samples = (np.sin(t) * 127).astype(np.int8)

            # 3. Copy data to memoryviews
            # memoryview handles slicing/assignment efficiently from numpy
            if ch1 is not None:
                ch1.cast('b')[:] = samples
            if ch2 is not None:
                ch2.cast('b')[:] = samples

            print(f"streamData size {size}")

            return False  # Return False if you want to CONTINUE streaming

        except Exception as e:
            print(f"NumPy Stream Error: {e}")
            return True # Stop on error


    def sentPack(self,client, ch1_size, ch2_size):
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

class ConfigCallbackImpl(streaming.ConfigCallback):

    def sigInt(self):
        obj.notifyStop()

# Creating a streaming client
confObj = streaming.ConfigStreamClient()
obj = streaming.DACStreamClient(confObj)

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
dac_rate = confObj.getConfig('dac_rate');
print("Current rate",dac_rate)


# Setting up network mode
confObj.sendConfig('dac_pass_mode','DAC_NET')

# Setting up a new decimation setting
confObj.sendConfig('dac_rate','125000000')

# Setting the memory block size
confObj.sendConfig('block_size','16384')

# Setting the size of reserved memory for DAC streaming
confObj.sendConfig('dac_size','1638400')

host = confObj.getHosts()[0]
if (obj.startStreamingFromMemorySink(host,True,True,streaming.DAC_8BIT)):
    print("Streaming is launched")
else:
    print("Error starting streaming")
    exit(1)

# Waiting for the streaming client to complete its work
obj.wait()

print("Send pack", callback.counter)
