import sys
import ctypes
from ctypes import *
import numpy as np

class MemoryInterface(object):
    def __init__(self, addr_base=0x00000000, parent_memory=None):
        self.addr_base = addr_base
        self._parent = parent_memory

    def read(self, addr):
        """Read one register """
        return self._parent.read(self.addr_base+addr)

    def reads(self, addr, length):
        return self._parent.reads(self.addr_base+addr, length)

    def write(self, addr, value):
        """Write one register """
        self._parent.write(self.addr_base+addr, value)

    def writes(self, addr, value):
        self._parent.writes(self.addr_base+addr, value)

    def setbit(self,addr,bitnumber):
        v = self.read(addr)|(0x00000001<<bitnumber)
        self.write(addr,v)
        return v

    def clrbit(self,addr,bitnumber):
        v = self.read(addr)&(~(0x00000001<<bitnumber))
        self.write(addr,v)
        return v

    def changebit(self, addr, bitnumber, v):
        if v:
            return self.setbit(addr, bitnumber)
        else:
            return self.clrbit(addr, bitnumber)

    def bitstate(self,addr,bitnumber):
        return bool(self.read(addr)&(0x00000001<<bitnumber))

    def to_pyint(self,v,bitlength=14):
        v = v & (2**bitlength-1)
        if v >> (bitlength-1):
            v = v - 2**bitlength
        return int(v)

    def from_pyint(self,v,bitlength=14):
        if v < 0:
            v = v + 2**bitlength
        v= (v & (2**bitlength-1))
        return int(v)



