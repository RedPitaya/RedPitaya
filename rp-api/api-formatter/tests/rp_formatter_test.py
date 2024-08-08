#!/usr/bin/python3

import math
from rp_formatter import *
import numpy as np

print("CFormatter(RP_F_WAV,44100)")
obj = CFormatter(RP_F_WAV,44100)
print(obj)

print("obj.setEndiannes(RP_F_LittleEndian)")
res = obj.setEndiannes(RP_F_LittleEndian)
print(res)

print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

N = 1024*16
arr_f = np.zeros(N, dtype=np.float32)
print("obj.setChannelFNP(RP_F_CH1,arr_f)")
res = obj.setChannelFNP(RP_F_CH1,arr_f)
print(res)

arr_d = np.zeros(N, dtype=np.double)
print("obj.setChannelDNP(RP_F_CH1,arr_d)")
res = obj.setChannelDNP(RP_F_CH1,arr_d)
print(res)

arr_ui8 = np.zeros(N, dtype=np.uint8)
print("obj.setChannelUI8NP(RP_F_CH1,arr_ui8)")
res = obj.setChannelUI8NP(RP_F_CH1,arr_ui8)
print(res)

arr_ui16 = np.zeros(N, dtype=np.uint16)
print("obj.setChannelUI16NP(RP_F_CH1,arr_ui16)")
res = obj.setChannelUI16NP(RP_F_CH1,arr_ui16)
print(res)

ch1 = arrFloat(1024)
ch2 = arrFloat(1024)
for n in range(0,1024,1):
    ch1[n] = math.sin(n/1024 * 3.141549 * 10)
    ch2[n] = math.cos(n/1024 * 3.141549 * 10)


print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj.openFile('test_float.wav')")
res = obj.openFile("test_float.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)


print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

ch1 = arrUInt8(1024)
ch2 = arrUInt8(1024)
for n in range(0,1024,1):
    ch1[n] = n % 255
    ch2[n] = n % 255


print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj.openFile('test_uint8.wav')")
res = obj.openFile("test_uint8.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)


print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

ch1 = arrUInt16(1024)
ch2 = arrUInt16(1024)
for n in range(0,1024,1):
    ch1[n] = n * 32
    ch2[n] = n * 32


print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj.openFile('test_uint16.wav')")
res = obj.openFile("test_uint16.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)


print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

ch1 = arrDouble(1024)
ch2 = arrDouble(1024)
for n in range(0,1024,1):
    ch1[n] = math.sin(n/1024 * 3.141549 * 10)
    ch2[n] = math.cos(n/1024 * 3.141549 * 10)



print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj.openFile('test_double.wav')")
res = obj.openFile("test_double.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)



ch1 = arrFloat(1024)
ch2 = arrFloat(1024)
ch3 = arrFloat(1024)
ch4 = arrFloat(1024)

for n in range(0,1024,1):
    ch1[n] = math.sin(n/1024 * 3.141549 * 10)
    ch2[n] = math.cos(n/1024 * 3.141549 * 10)
    ch3[n] = (n % 255) / 255
    ch4[n] = (n % 255) / -255

print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.openFile('test_1ch.wav')")
res = obj.openFile("test_1ch.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)



print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)


print("obj.openFile('test_2ch.wav')")
res = obj.openFile("test_2ch.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)


print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH3,ch3.cast(),1024)")
res = obj.setChannel(RP_F_CH3,ch3.cast(),1024)
print(res)


print("obj.openFile('test_3ch.wav')")
res = obj.openFile("test_3ch.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)


print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH3,ch3.cast(),1024)")
res = obj.setChannel(RP_F_CH3,ch3.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH4,ch4.cast(),1024)")
res = obj.setChannel(RP_F_CH4,ch4.cast(),1024)
print(res)


print("obj.openFile('test_4ch.wav')")
res = obj.openFile("test_4ch.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)


print("obj.resetWriter()")
res = obj.resetWriter()
print(res)

print("obj.clearBuffer()")
res = obj.clearBuffer()
print(res)

print("obj.setChannel(RP_F_CH1,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH1,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH3,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH3,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH4,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH4,ch2.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH5,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH5,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH6,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH6,ch2.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH7,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH7,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH8,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH8,ch2.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH9,ch1.cast(),1024)")
res = obj.setChannel(RP_F_CH9,ch1.cast(),1024)
print(res)

print("obj.setChannel(RP_F_CH10,ch2.cast(),1024)")
res = obj.setChannel(RP_F_CH10,ch2.cast(),1024)
print(res)



print("obj.openFile('test_10ch.wav')")
res = obj.openFile("test_10ch.wav")
print(res)

print("obj.writeToFile()")
res = obj.writeToFile()
print(res)

print("obj.closeFile()")
res = obj.closeFile()
print(res)


print("CFormatter(RP_F_TDMS,44100)")
obj2 = CFormatter(RP_F_TDMS,44100)
print(obj2)

print("obj2.clearBuffer()")
res = obj2.clearBuffer()
print(obj2)

print("obj2.setChannel(RP_F_CH1,ch1.cast(),1024),\"MATH\"")
res = obj2.setChannel(RP_F_CH1,ch1.cast(),1024,"MATH")
print(res)

print("obj2.setChannel(RP_F_CH2,ch2.cast(),1024)")
res = obj2.setChannel(RP_F_CH2,ch2.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH3,ch1.cast(),1024)")
res = obj2.setChannel(RP_F_CH3,ch1.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH4,ch2.cast(),1024)")
res = obj2.setChannel(RP_F_CH4,ch2.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH5,ch1.cast(),1024)")
res = obj2.setChannel(RP_F_CH5,ch1.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH6,ch2.cast(),1024)")
res = obj2.setChannel(RP_F_CH6,ch2.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH7,ch1.cast(),1024)")
res = obj2.setChannel(RP_F_CH7,ch1.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH8,ch2.cast(),1024)")
res = obj2.setChannel(RP_F_CH8,ch2.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH9,ch1.cast(),1024)")
res = obj2.setChannel(RP_F_CH9,ch1.cast(),1024)
print(res)

print("obj2.setChannel(RP_F_CH10,ch2.cast(),1024)")
res = obj2.setChannel(RP_F_CH10,ch2.cast(),1024)
print(res)

print("obj2.openFile('test_10ch.tdms')")
res = obj2.openFile("test_10ch.tdms")
print(res)

print("obj2.writeToFile()")
res = obj2.writeToFile()
print(res)

print("obj2.closeFile()")
res = obj2.closeFile()
print(res)


print("CFormatter(RP_F_CSV,44100)")
obj2 = CFormatter(RP_F_CSV,44100)
print(obj2)

print("obj2.clearBuffer()")
res = obj2.clearBuffer()
print(obj2)

print("obj2.setChannel(RP_F_CH1,ch1.cast(),1024),\"Channel 1\"")
res = obj2.setChannel(RP_F_CH1,ch1.cast(),1024,"Channel 1")
print(res)

print("obj2.setChannel(RP_F_CH2,ch2.cast(),1024),\"Channel 2\"")
res = obj2.setChannel(RP_F_CH2,ch2.cast(),1024,"Channel 2")
print(res)

print("obj2.setChannel(RP_F_CH3,ch1.cast(),1024),\"Channel 3\"")
res = obj2.setChannel(RP_F_CH3,ch1.cast(),1024,"Channel 3")
print(res)

print("obj2.setChannel(RP_F_CH4,ch2.cast(),1024),\"Channel 4\"")
res = obj2.setChannel(RP_F_CH4,ch2.cast(),1024,"Channel 4")
print(res)

print("obj2.setChannel(RP_F_CH5,ch1.cast(),1024),\"Channel 5\"")
res = obj2.setChannel(RP_F_CH5,ch1.cast(),1024,"Channel 5")
print(res)

print("obj2.setChannel(RP_F_CH6,ch2.cast(),1024),\"Channel 6\"")
res = obj2.setChannel(RP_F_CH6,ch2.cast(),1024,"Channel 6")
print(res)

print("obj2.setChannel(RP_F_CH7,ch1.cast(),1024),\"Channel 7\"")
res = obj2.setChannel(RP_F_CH7,ch1.cast(),1024,"Channel 7")
print(res)

print("obj2.setChannel(RP_F_CH8,ch2.cast(),1024),\"Channel 8\"")
res = obj2.setChannel(RP_F_CH8,ch2.cast(),1024,"Channel 8")
print(res)

print("obj2.setChannel(RP_F_CH9,ch1.cast(),1024),\"Channel 9\"")
res = obj2.setChannel(RP_F_CH9,ch1.cast(),1024,"Channel 9")
print(res)

print("obj2.setChannel(RP_F_CH10,ch2.cast(),1024),\"Channel 10\"")
res = obj2.setChannel(RP_F_CH10,ch2.cast(),1024,"Channel 10")
print(res)

print("obj2.openFile('test_10ch.csv')")
res = obj2.openFile("test_10ch.csv")
print(res)

print("obj2.writeToFile()")
res = obj2.writeToFile()
print(res)

print("obj2.closeFile()")
res = obj2.closeFile()
print(res)