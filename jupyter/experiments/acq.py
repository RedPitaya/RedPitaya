import os
import mmap
import numpy as np

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
        """Module instance index should be provided"""
        
        uio = uio+str(index)
        
        # open device file
        try:
            self.uio_dev = os.open(uio, os.O_RDWR | os.O_SYNC)
        except OSError as e:
            raise IOError(e.errno, "Opening {}: {}".format(uio, e.strerror))

        # exclusive lock
        try:
            fcntl.flock(self.uio_dev, fcntl.LOCK_EX)
        except IOError as e:
            raise IOError(e.errno, "Locking {}: {}".format(uio, e.strerror))

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