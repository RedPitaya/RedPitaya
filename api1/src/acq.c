from ctypes import *
import numpy as np
import math

class acq (object):
    CW = 31 #: counter size
    _CWr = 2**CW

    class _regset_t (Structure):
        _fields_ = [('cfg_pre', c_uint32),  # configuration pre  trigger
                    ('cfg_pst', c_uint32),  # configuration post trigger
                    ('sts_pre', c_uint32),  # status pre  trigger
                    ('sts_pst', c_uint32)]  # status post trigger

    def show_regset (self):
        """Print FPGA module register set for debugging purposes."""
        print (
            "cfg_pre = 0x{reg:08x} = {reg:10d}  # delay pre  trigger        \n".format(reg=self.regset.acq.cfg_pre)+
            "cfg_pst = 0x{reg:08x} = {reg:10d}  # delay post trigger        \n".format(reg=self.regset.acq.cfg_pst)+
            "sts_pre = 0x{reg:08x} = {reg:10d}  # status pre  trigger       \n".format(reg=self.regset.acq.sts_pre)+
            "sts_pst = 0x{reg:08x} = {reg:10d}  # status post trigger       \n".format(reg=self.regset.acq.sts_pst)
        )

    @property
    def trigger_pre (self) -> int:
        """Pre trigger delay.

        Number of samples stored into the buffer
        after start() before a trigger event is accepted.
        It makes sense for this number to be up to buffer size.
        """
        return (self.regset.acq.cfg_pre)

    @trigger_pre.setter
    def trigger_pre (self, value: int):
        if (value < self._CWr):
            self.regset.acq.cfg_pre = value
        else:
            raise ValueError("Pre trigger delay should be less or equal to {}.".format(self._CWr))

    @property
    def trigger_post (self) -> int:
        """Post trigger delay.

        Number of samples stored into the buffer
        after a trigger, before writing stops automatically.
        It makes sense for this number to be up to buffer size.
        """
        return (self.regset.acq.cfg_pst)

    @trigger_post.setter
    def trigger_post (self, value: int):
        if (value < self._CWr):
            self.regset.acq.cfg_pst = value
        else:
            raise ValueError("Post trigger delay should be less or equal to {}.".format(self._CWr))
        # TODO check range

    @property
    def trigger_pre_status (self) -> int:
        """Pre trigger sample counter status."""
        return (self.regset.acq.sts_pre & 0x7fffffff)

    @property
    def trigger_post_status (self) -> int:
        """Post trigger sample counter status."""
        return (self.regset.acq.sts_pst & 0x7fffffff)