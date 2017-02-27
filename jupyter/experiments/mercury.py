import os
import mmap
import numpy as np

import ctypes
from enum import Enum

class overlay (object):
    overlays = "/sys/kernel/config/device-tree/overlays"

    def __init__ (self, overlay:str):
        if not isinstance(overlay, str):
            raise TypeError("Bitstream name has to be a string.")

        if os.path.isfile("{}.dts".format(overlay)):
            self.overlay = overlay
            self.syspath = "{}/{}".format(self.overlays, self.overlay)
        else:
            raise IOError('Device tree overlay source {}.dts does not exist.'.format(self.overlay))

        os.system("dtc -I dts -O dtb -o {0}.dtbo -@ {0}.dts".format(self.overlay))
        os.system('cat /opt/redpitaya/fpga/{}/fpga.bit > /dev/xdevcfg'.format(self.overlay))
        
        if not os.path.isdir(self.syspath):
            os.system("mkdir {}".format(self.syspath))
        if not self.status():
            os.system("cat {}.dtbo > {}/dtbo".format(self.overlay, self.syspath))

    def __del__ (self):
        os.system("rmdir {}".format(self.syspath))
        del(self.syspath)
        del(self.overlay)

    def status (self):
        with open('{}/status'.format(self.syspath), 'r') as status_file:
            status_string = status_file.read()
            if   (status_string ==   "applied\n"): return (True)
            elif (status_string == "unapplied\n"): return (False)
            else:                                  return (None)


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
        # TODO add exclusive lock
        try:
            self.uio_dev = os.open(uio, os.O_RDWR | os.O_SYNC)
        except OSError as e:
            raise IOError(e.errno, "Opening {}: {}".format(uio, e.strerror))

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


class asg (object):
    # sampling frequency
    FS = 125000000.0
    # linear addition multiplication register width
    DW  = 14
    DWM = 14
    DWS = 14
    # fixed point range
    DWr  = (1 << (DW -1)) - 1
    DWMr = (1 << (DWM-2))
    DWSr = (1 << (DWS-1)) - 1
    # buffer parameters
    CWM = 14  # counter width magnitude (fixed point integer)
    CWF = 16  # counter width fraction  (fixed point fraction)
    N = 2**CWM # table size
    # control register masks
    CTL_STO_MASK = np.uint32(1<<3) # 1 - stop/abort; returns 1 when stopped
    CTL_STA_MASK = np.uint32(1<<2) # 1 - start
    CTL_SWT_MASK = np.uint32(1<<1) # 1 - sw trigger bit (sw trigger must be enabled)
    CTL_RST_MASK = np.uint32(1<<0) # 1 - reset state machine so that it is in known state
    
    regset_dtype = np.dtype([
        # control/status
        ('ctl_sts', 'uint32'),
        # trigger configuration
        ('cfg_trg', 'uint32'),  # trigger mask
        ('rsv0'   , 'uint32', 2),  # reserved
        # buffer configuration
        ('cfg_siz', 'uint32'),  # size
        ('cfg_off', 'uint32'),  # offset
        ('cfg_stp', 'uint32'),  # step
        ('rsv1'   , 'uint32'),  # reserved
        # burst mode
        ('cfg_bmd', 'uint32'),  # mode [1:0] = [inf, ben]
        ('cfg_bdl', 'uint32'),  # data length
        ('cfg_bln', 'uint32'),  # length (data+pause)
        ('cfg_bnm', 'uint32'),  # number of bursts pulses
        # burst status
        ('sts_bln', 'uint32'),  # length (current position inside burst length)
        ('sts_bnm', 'uint32'),  # number (current burst counter)
        # linear transformation
        ('cfg_mul',  'int32'),  # multiplier (amplitude)
        ('cfg_sum',  'int32')   # addedr (offset)
    ])

    def __init__ (self, index:int, uio:str = '/dev/uio/asg'):
        uio = uio+str(index)
        # TODO add exclusive lock
        try:
            self.uio_dev = os.open(uio, os.O_RDWR | os.O_SYNC)
        except OSError as e:
            raise IOError(e.errno, "Opening {}: {}".format(uio, e.strerror))

        # map regset
        try:
            self.uio_reg = mmap.mmap(
                fileno=self.uio_dev, length=mmap.PAGESIZE, offset=0x0,
                flags=mmap.MAP_SHARED, prot=(mmap.PROT_READ | mmap.PROT_WRITE))
        except OSError as e:
            raise IOError(e.errno, "Mapping (regset) {}: {}".format(uio, e.strerror))

        regset_array = np.recarray(1, self.regset_dtype, buf=self.uio_reg)
        self.regset = regset_array[0]
        
        # map buffer table
        try:
            self.uio_tbl = mmap.mmap(
                # TODO: probably the length should be rounded up to mmap.PAGESIZE
                fileno=self.uio_dev, length=4*self.N, offset=mmap.PAGESIZE,
                flags=mmap.MAP_SHARED, prot=(mmap.PROT_READ | mmap.PROT_WRITE))
        except OSError as e:
            raise IOError(e.errno, "Mapping (buffer) {}: {}".format(uio, e.strerror))

        #table_array = np.recarray(1, self.table_dtype, buf=self.uio_tbl)
        self.table = np.frombuffer(self.uio_tbl, 'int32')

    def __del__ (self):
        self.uio_tbl.close()
        self.uio_reg.close()
        try:
            os.close(self.uio_dev)
        except OSError as e:
            raise IOError(e.errno, "Closing {}: {}".format(uio, e.strerror))

    def show_regset (self):
        print (
            "ctl_sts = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.ctl_sts)+
            "cfg_trg = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_trg)+
            "cfg_siz = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_siz)+
            "cfg_off = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_off)+
            "cfg_stp = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_stp)+
            "cfg_bmd = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_bmd)+
            "cfg_bdl = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_bdl)+
            "cfg_bnm = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_bnm)+
            "sts_bln = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.sts_bln)+
            "sts_bnm = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.sts_bnm)+
            "cfg_mul = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_mul)+
            "cfg_sum = 0x{reg:x} = {reg:d}\n".format(reg=self.regset.cfg_sum)
        )

    def reset (self):
        # reset state machine
        self.regset.ctl_sts = self.CTL_RST_MASK

    def trigger (self):
        # activate SW trigger
        self.regset.ctl_sts = self.CTL_SWT_MASK

    def status (self) -> bool:
        # start state machine
        return bool(self.regset.ctl_sts & self.CTL_STA_MASK)

    @property
    def amplitude (self) -> float:
        """Output amplitude in Vols"""
        return (self.regset.cfg_mul / self.DWMr)
    
    @amplitude.setter
    def amplitude (self, value: float):
        """Output amplitude in Vols"""
        # TODO: fix saturation
        if (-1.0 <= value <= 1.0):
            self.regset.cfg_mul = value * self.DWMr
        else:
            raise ValueError("Output amplitude should be inside [-1,1]")
    
    @property
    def offset (self) -> float:
        """Output offset in Vols"""
        return (self.regset.cfg_sum / self.DWSr)
    
    @offset.setter
    def offset (self, value: float):
        """Output offset in Vols"""
        # TODO: fix saturation
        if (-1.0 <= value <= 1.0):
            self.regset.cfg_sum = value * self.DWSr
        else:
            raise ValueError("Output offset should be inside [-1,1]")

    @property
    def frequency (self) -> float:
        """Frequency in Hz"""
        siz = self.regset.cfg_siz + 1
        stp = self.regset.cfg_stp + 1
        return (stp / siz * self.FS)
    
    @frequency.setter
    def frequency (self, value: float):
        """Frequency in Hz"""
        if (value < self.FS/2):
            siz = self.regset.cfg_siz + 1
            self.regset.cfg_stp = int(siz * (value / self.FS)) - 1
        else:
            raise ValueError("Frequency should be less then half the sample rate. f < FS/2 = {}".format(self.FS/2))
    
    @property
    def phase (self) -> float:
        """Phase in angular degrees"""
        siz = self.regset.cfg_siz + 1
        off = self.regset.cfg_off
        return (stp / siz * 360)
    
    @phase.setter
    def phase (self, value: float):
        """Phase in angular degrees"""
        # TODO add range check
        siz = self.regset.cfg_siz + 1
        self.regset.cfg_off = int(siz * (value % 360) / 360)

    @property
    def waveform (self):
        """Waveworm table containing normalized values in the range [-1,1]"""
        siz = (self.regset.cfg_siz + 1) >> self.CWF
        # TODO: nparray
        return [self.table[i] / self.DWr for i in range(siz)]
    
    @waveform.setter
    def waveform (self, value):
        """Waveworm table containing normalized values in the range [-1,1]"""
        # TODO check table size shape
        siz = len(value)
        if (siz <= self.N):
            for i in range(siz):
                # TODO add saturation
                self.table[i] = int(value[i] * self.DWr)
            self.regset.cfg_siz = (siz << self.CWF) - 1
        else:
            raise ValueError("Waveform table size should not excede buffer size. N = {}".format(self.N))

    class modes(Enum):
        CONTINUOUS = ctypes.c_uint32(0x0)
        BURST_FIN  = ctypes.c_uint32(0x1)
        BURST_INF  = ctypes.c_uint32(0x3)
    
    @property
    def trigger_mask (self):
        return (modes(self.regset.cfg_trg))

    @trigger_mask.setter
    def trigger_mask (self, value):
        # TODO check range
        self.regset.cfg_trg = value

    @property
    def mode (self):
        return (modes(self.regset.cfg_bst))

    @mode.setter
    def mode (self, value):
        # TODO check range
        self.regset.cfg_bst = value

    @property
    def burst_repetitions (self) -> int:
        return (self.regset.cfg_bnm)

    @burst_repetitions.setter
    def burst_repetitions (self, value: int):
        # TODO check range
        self.regset.cfg_bnm = value
        
    @property
    def burst_data_len (self) -> int:
        return (self.regset.cfg_bdl)

    @burst_data_len.setter
    def burst_data_len (self, value: int):
        # TODO check range
        self.regset.cfg_bdl = value
        
    @property
    def burst_period_len (self) -> int:
        return (self.regset.cfg_bln)

    @burst_period_len.setter
    def burst_period_len (self, value: int):
        # TODO check range
        self.regset.cfg_bln = value


class acq (object):
    # sampling frequency
    FS = 125000000.0
    # linear addition multiplication register width
    DW  = 14
    DWM = 14
    DWS = 14
    # fixed point range
    DWr  = (1 << (DW -1)) - 1
    DWMr = (1 << (DWM-2))
    DWSr = (1 << (DWS-1)) - 1
    # buffer parameters
    CWM = 14  # counter width magnitude (fixed point integer)
    CWF = 16  # counter width fraction  (fixed point fraction)
    N = 2**CWM # table size
    # control register masks
    CTL_STO_MASK = np.uint32(1<<3) # 1 - stop/abort; returns 1 when stopped
    CTL_STA_MASK = np.uint32(1<<2) # 1 - start
    CTL_SWT_MASK = np.uint32(1<<1) # 1 - sw trigger bit (sw trigger must be enabled)
    CTL_RST_MASK = np.uint32(1<<0) # 1 - reset state machine so that it is in known state
    
    regset_dtype = np.dtype([
        # control/status
        ('ctl_sts', 'uint32'),
        ('cfg_mod', 'uint32'),  # mode
        # trigger configuration
        ('cfg_trg', 'uint32'),  # trigger mask
        ('rsv0'   , 'uint32'),  # reserved
        # pre/post trigger counters
        ('cfg_pre', 'uint32'),  # configuration pre  trigger
        ('cfg_pst', 'uint32'),  # configuration post trigger
        ('sts_pre', 'uint32'),  # status pre  trigger
        ('sts_pst', 'uint32'),  # status post trigger
        # timestamp
        ('cts_acq', 'uint32',2),  # start
        ('cts_trg', 'uint32',2),  # trigger
        ('cts_stp', 'uint32',2),  # stop
        ('rsv1'   , 'uint32',2),  # reserved
        # edge detection
        ('cfg_lvl', 'uint32'),  # level
        ('cfg_hst', 'uint32'),  # hysteresis
        ('cfg_edg', 'uint32'),  # edge (0-pos, 1-neg)
        ('cfg_rng', 'uint32'),  # range (not used by HW)

        # decimation
        ('cfg_byp', 'uint32'),  # bypass
        ('cfg_dec', 'uint32'),  # decimation factor
        ('cfg_shr', 'uint32'),  # shift right
        # filter
        ('cfg_avg', 'uint32'),  # average enable
        ('cfg_faa', 'uint32'),  # AA coeficient
        ('cfg_fbb', 'uint32'),  # BB coeficient
        ('cfg_fkk', 'uint32'),  # KK coeficient
        ('cfg_fpp', 'uint32')   # PP coeficient
    ])

    def __init__ (self, index:int, uio:str = '/dev/uio/acq'):
        uio = uio+str(index)
        # TODO add exclusive lock
        try:
            self.uio_dev = os.open(uio, os.O_RDWR | os.O_SYNC)
        except OSError as e:
            raise IOError(e.errno, "Opening {}: {}".format(uio, e.strerror))

        # map regset
        try:
            self.uio_reg = mmap.mmap(
                fileno=self.uio_dev, length=mmap.PAGESIZE, offset=0x0,
                flags=mmap.MAP_SHARED, prot=(mmap.PROT_READ | mmap.PROT_WRITE))
        except OSError as e:
            raise IOError(e.errno, "Mapping (regset) {}: {}".format(uio, e.strerror))

        regset_array = np.recarray(1, self.regset_dtype, buf=self.uio_reg)
        self.regset = regset_array[0]
        
        # map buffer table
        try:
            self.uio_tbl = mmap.mmap(
                # TODO: probably the length should be rounded up to mmap.PAGESIZE
                fileno=self.uio_dev, length=4*self.N, offset=mmap.PAGESIZE,
                flags=mmap.MAP_SHARED, prot=(mmap.PROT_READ | mmap.PROT_WRITE))
        except OSError as e:
            raise IOError(e.errno, "Mapping (buffer) {}: {}".format(uio, e.strerror))

        #table_array = np.recarray(1, self.table_dtype, buf=self.uio_tbl)
        self.table = np.frombuffer(self.uio_tbl, 'int32')

    def __del__ (self):
        self.uio_tbl.close()
        self.uio_reg.close()
        try:
            os.close(self.uio_dev)
        except OSError as e:
            raise IOError(e.errno, "Closing {}: {}".format(uio, e.strerror))

