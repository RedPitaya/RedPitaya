import os
import fcntl
import mmap

import numpy as np

class hwid ():
    regset_dtype = np.dtype([
        ('hwid' , 'uint32'),
        ('rsv0' , 'uint32'),  # reserved
        ('efuse', 'uint32'),
        ('rsv1' , 'uint32'),  # reserved
        ('dna'  , 'uint32', 2),
        ('rsv3' , 'uint32', 2),  # reserved
        ('gith' , 'uint32', 5)
    ])

    def __init__ (self, uio:str = '/dev/uio/hwid'):

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
    def hwid (self):
        return (self.regset.hwid)

    @property
    def efuse (self):
        return (self.regset.efuse)

    @property
    def dna (self):
        return ((self.regset.dna[1] << 32) | self.regset.dna[0])

    @property
    def gith (self):
        return (''.join(["{:08x}".format(regid.regset.gith[i]) for i in range(5)]))
