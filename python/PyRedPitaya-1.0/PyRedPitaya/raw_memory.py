import sys
import ctypes
from ctypes import *
import numpy as np
from time import time

if 'PyRedPitayaTest' in sys.modules.keys():
    from PyRedPitayaTest import libmonitor_file
else:
    libmonitor_file = 'libmonitor.so'

libmonitor = ctypes.cdll.LoadLibrary(libmonitor_file)
libmonitor.read_value.restype = c_uint32

#TODO
#Specify output types of read_value to uint32

class BoardRawMemory(object):
    """ Classes uses to interface de RedPitaya memory

    This is a one to one match to the libmonitor.so library"""
    a = libmonitor

    def read(self, addr):
        """ Read one word in the memory """
        return self.a.read_value(addr)

    def reads(self, addr, length, return_buffer=False):
        """ Read length words in the memory

        output : either an array on uint32 or a string buffer"""
#        t0 = time()
        buf = create_string_buffer(sizeof(ctypes.c_uint32)*length)
        self.a.read_values(addr,cast(buf, POINTER(ctypes.c_uint32)),length)
#        print "Time :",time()-t0
        if return_buffer:
            return buf.raw
        else:
            return np.frombuffer(buf.raw, dtype='uint32')

    def write(self, addr, value):
        """ Write one word in the memory """
        self.a.write_value(addr, value)

    def writes(self, addr, values):
        """Write length words in memory

        input : values should be a list, np.array of numbers or a str (interpreted as a string buffer)"""
        if not isinstance(values, str):
            values = np.array(values, dtype='uint32')
            values = str(values.data)
        buf = create_string_buffer(values)
        self.a.write_values(addr, cast(buf, POINTER(ctypes.c_uint32)), len(values)//4)
