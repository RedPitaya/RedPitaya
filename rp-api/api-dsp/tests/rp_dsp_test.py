#!/usr/bin/python3


import rp_dsp

# See the logic for working with this class in the file: /Test/spectrum/spectrum.cpp

print("rp_dsp.CDSP(2,256,125000000)")
obj = rp_dsp.CDSP(2,256,125000000)
print(obj)

print("obj.setChannel(0,True)")
res = obj.setChannel(0,True)
print(res)

print("obj.setChannel(1,True)")
res = obj.setChannel(1,True)
print(res)

print("obj.setSignalLength(256)")
res = obj.setSignalLength(256)
print(res)

print("obj.getSignalLength()")
res = obj.getSignalLength()
print(res)

print("obj.getSignalMaxLength()")
res = obj.getSignalMaxLength()
print(res)

print("obj.getOutSignalLength()")
res = obj.getOutSignalLength()
print(res)

print("obj.getOutSignalMaxLength()")
out_signal = obj.getOutSignalMaxLength()
print(out_signal)

print("obj.window_init(rp_dsp.FLAT_TOP)")
res = obj.window_init(rp_dsp.FLAT_TOP)
print(res)

print("obj.getCurrentWindowMode()")
res = obj.getCurrentWindowMode()
print(res)

print("obj.setImpedance(100.0)")
res = obj.setImpedance(100.0)
print(res)

print("obj.getImpedance()")
res = obj.getImpedance()
print(res)

print("obj.setRemoveDC(True)")
res = obj.setRemoveDC(True)
print(res)

print("obj.getRemoveDC()")
res = obj.getRemoveDC()
print(res)

print("obj.setMode(True)")
res = obj.setMode(True)
print(res)

print("obj.getMode()")
res = obj.getMode()
print(res)

print("obj.fftInit()")
res = obj.fftInit()
print(res)

print("obj.createData()")
data = obj.createData()
print(data)

print("rp_dsp.arrpDouble.frompointer(data.m_in)")
pdata_in = rp_dsp.arrpDouble.frompointer(data.m_in)
print(pdata_in)

print("rp_dsp.arrDouble.frompointer(pdata_in[0])")
data_in_ch1 = rp_dsp.arrDouble.frompointer(pdata_in[0])
print(data_in_ch1)

print("rp_dsp.arrDouble.frompointer(pdata_in[1])")
data_in_ch2 = rp_dsp.arrDouble.frompointer(pdata_in[1])
print(data_in_ch2)

data_in_ch1[0] = 0
data_in_ch1[1] = 1
data_in_ch1[2] = 2
data_in_ch1[3] = 3

print(data_in_ch1[0])
print(data_in_ch1[1])
print(data_in_ch1[2])
print(data_in_ch1[3])


print("obj.prepareFreqVector(data,125000000,1)")
res = obj.prepareFreqVector(data,125000000,1)
print(res)


print("obj.windowFilter(data)")
res = obj.windowFilter(data)
print(res)

print("obj.fft(data)")
res = obj.fft(data)
print(res)

print("obj.decimate(data,out_signal,out_signal)")
res = obj.decimate(data,out_signal,out_signal)
print(res)

print("obj.cnvToDBM(data,1)")
res = obj.cnvToDBM(data,1)
print(res)

print("obj.cnvToDBMMaxValueRanged(data,1,0,100000)")
res = obj.cnvToDBMMaxValueRanged(data,1,0,100000)
print(res)

print("obj.cnvToMetric(data,1)")
res = obj.cnvToMetric(data,1)
print(res)

print("obj.remoteDCCount()")
res = obj.remoteDCCount()
print(res)

print("obj.window_clean()")
res = obj.window_clean()
print(res)

print("obj.deleteData()")
res = obj.deleteData(data)
print(res)

print("obj.fftClean()")
res = obj.fftClean()
print(res)


# Optimized math functions on arm neon
# These functions can give a calculation error

print("rp_dsp.log10f_neon(10000)")
res = rp_dsp.log10f_neon(10000)
print(res)

print("rp_dsp.sqrtf_neon(100)")
res = rp_dsp.sqrtf_neon(100)
print(res)