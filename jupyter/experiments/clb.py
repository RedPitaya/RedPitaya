import os
import fcntl
import mmap

import numpy as np

class clb (object):
    DWA = 16
    DWG = 14
    DWAr = 2**DWA - 1
    DWGr = 2**DWG - 1
    DWA1 = 2**(DWA-1)
    DWG1 = 2**(DWG-1)
    
    regset_channel_dtype = np.dtype([
        ('mul', 'int32'),  # multiplication
        ('sum', 'int32')   # summation
    ])

    regset_dtype = np.dtype([
        ('adc', regset_channel_dtype, 2),  # oscilloscope
        ('dac', regset_channel_dtype, 2),  # generator
    ])

    clb_channel_dtype = np.dtype([
        ('mul', 'float32'),  # multiplication
        ('sum', 'float32')   # summation
    ])

    clb_dtype = np.dtype([
        ('adc', clb_channel_dtype, 2),  # oscilloscope
        ('dac', clb_channel_dtype, 2),  # generator
    ])

    def __init__ (self, uio:str = '/dev/uio/clb'):

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

        tmp_array = np.recarray(1, self.clb_dtype)
        self.rmp = tmp_array[0]

    def __del__ (self):
        self.uio_mem.close()
        try:
            os.close(self.uio_dev)
        except OSError as e:
            raise IOError(e.errno, "Closing {}: {}".format(uio, e.strerror))

    def get_adc_gain (self, ch: int) -> float:
        return (self.regset.adc[ch].mul / DWA1)

    def set_adc_gain (self, ch: int, gain: float):
        self.regset.adc[ch].mul = gain * DWA1

    def get_adc_amplitude (self, ch: int) -> float:
        return (self.regset.adc[ch].sum / DWAr)

    def set_adc_amplitude (self, ch: int, amplitude: float):
        self.regset.adc[ch].mul = gain * DWAr

    def get_dac_gain (self, ch: int) -> float:
        return (self.regset.dac[ch].mul / DWG1)

    def set_dac_gain (self, ch: int, gain: float):
        self.regset.dac[ch].mul = gain * DWG1

    def get_dac_amplitude (self, ch: int) -> float:
        return (self.regset.dac[ch].sum / DWGr)

    def set_dac_amplitude (self, ch: int, amplitude: float):
        self.regset.dac[ch].mul = gain * DWGr

    def eeprom_read (self):
        pass