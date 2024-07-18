#!/usr/bin/python3

from rp_arb import *
import numpy as np

print("rp_ARBInit()")
res = rp_ARBInit()
print(res)

print("rp_ARBLoadFiles()")
res = rp_ARBLoadFiles()
print(res)

print("rp_ARBGenFile('')")
res = rp_ARBGenFile('')
print(res)

print("rp_ARBGetCount()")
res = rp_ARBGetCount()
print(res)

s = ""
print("rp_ARBGetName(0)")
res = rp_ARBGetName(0)
print(res)

print("rp_ARBGetFileName(0)")
res = rp_ARBGetFileName(0)
print(res)

# Max 16k samples
d = arrFloat(1024 * 16)
print("rp_ARBGetSignal(0,d.cast())")
res = rp_ARBGetSignal(0,d.cast())
print(res)

d = arrFloat(1024 * 16)
print("rp_ARBGetSignalByName('',d.cast())")
res = rp_ARBGetSignalByName('',d.cast())
print(res)

N = 1024*16
arr_f = np.zeros(N, dtype=np.float32)

print("rp_ARBGetSignalNP(0,arr_f)")
res = rp_ARBGetSignalNP(0,arr_f)
print(res)

print("rp_ARBGetSignalByNameNP('',arr_f)")
res = rp_ARBGetSignalByNameNP('',arr_f)
print(res)

print("rp_ARBSetColor(0,123)")
res = rp_ARBSetColor(0,123)
print(res)

print("rp_ARBGetColor(0)")
res = rp_ARBGetColor(0)
print(res)

print("rp_ARBRenameFile(0,'')")
res = rp_ARBRenameFile(0,'')
print(res)

print("rp_ARBLoadToFPGA(0,'')")
res = rp_ARBLoadToFPGA(0,'')
print(res)

print("rp_ARBIsValid('')")
res = rp_ARBIsValid('')
print(res)
