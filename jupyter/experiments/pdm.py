import os
import fcntl
import mmap

import numpy as np

class pdm ():

    DN = 4
    DW = 8
    DWr = 2**DW
    V = 1.8  # voltage

    regset_dtype = np.dtype([
        ('pdm' , 'uint32', self.DN)
    ])

    def __init__ (self, uio:str = '/dev/uio/pdm'):

        # open device file
        try:
            self.uio_dev = os.open(uio, os.O_RDWR | os.O_SYNC)
        except OSError as e:
            raise IOError(e.errno, "Opening {}: {}".format(uio, e.strerror))

        # exclusive lock
        try:
            fcntl.flock(self.uio_dev, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except IOError as e:
            raise IOError(e.errno, "Locking {}: {}".format(uio, e.strerror))

        # map regset
        try:
            self.uio_mem = mmap.mmap(
                fileno=self.uio_dev, length=mmap.PAGESIZE, offset=0x0,
                flags=mmap.MAP_SHARED, prot=(mmap.PROT_READ | mmap.PROT_WRITE))
        except OSError as e:
            raise IOError(e.errno, "Mapping {}: {}".format(uio, e.strerror))

        regset_array = np.recarray(1, self.regset_dtype, buf=self.uio_mem)
        self.regset = regset_array[0]

    def __del__ (self):
        self.uio_mem.close()
        try:
            os.close(self.uio_dev)
        except OSError as e:
            raise IOError(e.errno, "Closing {}: {}".format(uio, e.strerror))

    @property
    def pdm (self) -> tuple:
        return ([self.regset.pdm[i] / self.DWr * self.V for i in range(self.DN))

    @pdm.setter
    def pdm (self, value: tuple):
         for i in range(self.DN):
             self.regset.pdm[i] = value[i] / self.V * self.DWr
