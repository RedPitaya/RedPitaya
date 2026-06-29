#!/usr/bin/python3

import streaming
import math
import time
import numpy as np

class Callback(streaming.DACCallback):
    counter = 0

    def streamData16Bit(self, client, ch1, ch2, size):
        try:
            min_val = 0
            mid_val = 3000
            max_val = 6000
            repeat_start = 4
            repeat_end = 4

            if size < repeat_start + repeat_end:
                print(f"Error: size {size} too small")
                return True

            samples = np.empty(size, dtype=np.int16)

            samples[0:repeat_start] = min_val

            samples[size - repeat_end:size] = max_val

            middle_start = repeat_start
            middle_end = size - repeat_end
            middle_size = middle_end - middle_start

            if middle_size > 0:
                pattern = np.array([max_val, min_val, mid_val], dtype=np.int16)
                repeats = (middle_size // 3) + 1
                middle_pattern = np.tile(pattern, repeats)[:middle_size]

                if middle_size > 0:
                    if middle_pattern[-1] != min_val:
                        middle_pattern[-1] = min_val
                        if middle_size > 1 and middle_pattern[-2] == min_val:
                            middle_pattern[-2] = max_val
                        elif middle_size > 2 and middle_pattern[-2] == mid_val and middle_pattern[-3] == min_val:
                            middle_pattern[-2] = max_val

                samples[middle_start:middle_end] = middle_pattern

            if ch1 is not None:
                ch1.cast('h')[:] = samples

            if ch2 is not None:
                ch2.cast('h')[:] = samples

            print(f"streamData size {size}")
            print(f"Buffer: {samples.tolist()}")
            time.sleep(1)
            return False

        except Exception as e:
            print(f"NumPy Stream Error: {e}")
            return True




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
confObj.sendConfig('dac_rate','7812500')

# Setting the memory block size
#confObj.sendConfig('block_size','131072')
confObj.sendConfig('block_size','1024')

# Setting the size of reserved memory for DAC streaming
confObj.sendConfig('dac_size','16384000')

host = confObj.getHosts()[0]
if (obj.startStreamingFromMemorySink(host,True,True,streaming.DAC_16BIT)):
    print("Streaming is launched")
else:
    print("Error starting streaming")
    exit(1)

# Waiting for the streaming client to complete its work
obj.wait()

print("Send pack", callback.counter)
