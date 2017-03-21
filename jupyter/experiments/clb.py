import os
import fcntl
import mmap

import numpy as np

class clb (object):
    DWA = 16
    DWG = 14
    DWAr = 2**DWA - 1
    DWGr = 2**DWG - 1
    DWA1 = 2**(DWA-2)
    DWG1 = 2**(DWG-2)
    
    MAGIC = 0xAABBCCDD
    eeprom_device = "/sys/bus/i2c/devices/0-0050/eeprom"
    eeprom_offset = 0x0008

    # FPGA regset structure
    regset_channel_dtype = np.dtype([
        ('ctl_mul', 'int32'),  # multiplication
        ('ctl_sum', 'int32')   # summation
    ])
    regset_dtype = np.dtype([
        ('adc', regset_channel_dtype, 2),  # oscilloscope
        ('dac', regset_channel_dtype, 2),  # generator
    ])

    # floating point structure
    clb_channel_dtype = np.dtype([
        ('gain'  , 'float32'),  # multiplication
        ('offset', 'float32')   # summation
    ])
    clb_dtype = np.dtype([
        ('adc', clb_channel_dtype, 2),  # oscilloscope
        ('dac', clb_channel_dtype, 2),  # generator
    ])

    # EEPROM structure
    eeprom_dtype = np.dtype([
        ('adc_hi_gain', 'uint32', 2),
        ('adc_lo_gain', 'uint32', 2),
        ('adc_lo_off' ,  'int32', 2),
        ('dac_gain'   , 'uint32', 2),
        ('dac_off'    ,  'int32', 2),
        ('magic'      , 'uint32'),
        ('adc_hi_off' ,  'int32', 2)
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
        self.tmp = tmp_array[0]

    def __del__ (self):
        self.uio_mem.close()
        try:
            os.close(self.uio_dev)
        except OSError as e:
            raise IOError(e.errno, "Closing {}: {}".format(uio, e.strerror))

    def get_adc_gain (self, ch: int) -> float:
        return (self.regset.adc[ch].ctl_mul / self.DWA1)

    def set_adc_gain (self, ch: int, gain: float):
        self.regset.adc[ch].ctl_mul = int(gain * self.DWA1)

    def get_adc_offset (self, ch: int) -> float:
        return (self.regset.adc[ch].ctl_sum / self.DWAr)

    def set_adc_offset (self, ch: int, offset: float):
        self.regset.adc[ch].ctl_mul = int(offset * self.DWAr)

    def get_dac_gain (self, ch: int) -> float:
        return (self.regset.dac[ch].ctl_mul / self.DWG1)

    def set_dac_gain (self, ch: int, gain: float):
        self.regset.dac[ch].ctl_mul = int(gain * self.DWG1)

    def get_dac_offset (self, ch: int) -> float:
        return (self.regset.dac[ch].ctl_sum / self.DWGr)

    def set_dac_offset (self, ch: int, offset: float):
        self.regset.dac[ch].ctl_mul = int(offset * self.DWGr)

    def eeprom_read (self):
        # open EEPROM device
        try:
            eeprom_file = open(self.eeprom_device, 'rb')
        except OSError as e:
            raise IOError(e.errno, "Opening {}: {}".format(uio, e.strerror))

        # seek to calibration data
        try:
            eeprom_file.seek(self.eeprom_offset)
        except IOError as e:
            raise IOError(e.errno, "Seek {}: {}".format(uio, e.strerror))

        try:
            buffer = eeprom_file.read (self.eeprom_dtype.itemsize)
        except IOError as e:
            raise IOError(e.errno, "Read {}: {}".format(uio, e.strerror))

        try:
            eeprom_file.close()
        except IOError as e:
            raise IOError(e.errno, "Close {}: {}".format(uio, e.strerror))

        # map buffer onto structure
        eeprom_array = np.recarray(1, self.eeprom_dtype, buf=buffer)
        eeprom_struct = eeprom_array[0]
    
        if (eeprom_struct.magic != self.MAGIC):
            channels = [0, 1]
            for ch in channels:
                eeprom_struct.adc_hi_off[ch] = eeprom_struct.adc_lo_off[ch];

        self.eeprom_struct = eeprom_struct