#!/usr/bin/python3

import rp_la
import numpy as np


class Callback(rp_la.CLACallback):
    def captureStatus(self,controller,isTimeout,bytes,samples,preTrig, postTrig):
        print("captureStatus timeout =",isTimeout,"bytes =",bytes,"samples =",samples,"preTrig =",preTrig,"postTrig =",postTrig)

    def decodeDone(self,controller,name):
        print("Decode done ",name)


obj = rp_la.CLAController()

callback = Callback()
obj.setDelegate(callback.__disown__())

obj.setEnableRLE(True)
obj.setDecimation(1)
# obj.setTrigger(0,rp_la.LA_RISING_OR_FALLING)
obj.setPreTriggerSamples(750000)
obj.setPostTriggerSamples(750000)

obj.addDecoder("UART",rp_la.LA_DECODER_UART)
obj.addDecoder("I2C",rp_la.LA_DECODER_I2C)
obj.addDecoder("SPI",rp_la.LA_DECODER_SPI)
obj.addDecoder("CAN",rp_la.LA_DECODER_CAN)

obj.runAsync(0)
print("Start wait")
obj.wait(0)
print("End wait")
obj.saveCaptureDataToFile("/tmp/data.bin")
rawBytesCount = obj.getCapturedDataSize()
arr_ui8 = np.zeros(rawBytesCount, dtype=np.uint8)
print("Packed samples count:",obj.getDataNP(arr_ui8))
arr2_ui8 = np.zeros(obj.getCapturedSamples(), dtype=np.uint8)
print("Unpacked samples count:",obj.getUnpackedRLEDataNP(arr2_ui8))
print("RLE DATA", arr_ui8)
print("UNPACKED DATA",arr2_ui8)
obj.printRLE(False)


json=obj.getDefaultSettings(rp_la.LA_DECODER_CAN)
print(json)
decode=obj.decodeNP(rp_la.LA_DECODER_CAN,json, arr_ui8)
print(decode)
del obj
