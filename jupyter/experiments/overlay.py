import os

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
