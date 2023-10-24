import os
import time


class overlay(object):
    """Class handling device tree overlays and FPGA bitstreams."""
    overlays = "/sys/kernel/config/device-tree/overlays"
    fpgapath = "/opt/redpitaya/fpga"
    overlaysh = "/opt/redpitaya/sbin/overlay.sh"

    def __init__(self, overlay: str = 'v0.94'):
        if not isinstance(overlay, str):
            raise TypeError("Bitstream name has to be a string.")

        if os.path.isfile(self.overlaysh):
            self.overlay = overlay
            self.syspath = "{}/{}".format(self.overlays, self.overlay)

        if self.status():
            print('Check FPGA [OK].')
            return

        os.system("{} {}".format(self.overlaysh,overlay))
        # this delay makes sure all devices are created before continuing
        time.sleep(0.5)
        if self.status():
            print('Load overlay [OK].')
        else:
            print('Load overlay [FAIL].')

    def __del__(self):
        print('Overlay __del__ was activated.')
        print('But since there are garbage collection ordering issues, this code is commented out.')
#        os.system("rmdir {}".format(self.syspath))
#        del(self.syspath)
#        del(self.overlay)

    def status(self) -> bool:
        """Checking device tree overlay status.

        :returns: device tree overlay 'applied' status
        :rtype: bool
        """
        if not os.path.isfile('/tmp/loaded_fpga.inf'):
            return False
        else:
            with open('/tmp/loaded_fpga.inf', 'r') as status_file:
                status_string = status_file.read()
                if   (status_string !=   self.overlay): return False

        if not os.path.isfile('{}/Full/status'.format(self.overlays)):
            return False
        else:
            with open('{}/Full/status'.format(self.overlays), 'r') as status_file:
                status_string = status_file.read()
                if   (status_string ==   "applied\n"): return True
                else:                                  return False
