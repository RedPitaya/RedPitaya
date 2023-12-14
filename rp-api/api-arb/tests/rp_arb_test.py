#!/usr/bin/python3

from rp_arb import *

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

print("rp_ARBRenameFile(0,'')")
res = rp_ARBRenameFile(0,'')
print(res)

print("rp_ARBLoadToFPGA(0,'')")
res = rp_ARBLoadToFPGA(0,'')
print(res)