import os

class overlay (object):
    overlays = "/sys/kernel/config/device-tree/overlays"

    def __init__ (self, overlay:str):
        self.overlay = overlay

        if not isinstance(overlay, str):
            raise TypeError("Bitstream name has to be a string.")

        if os.path.isfile("{}.dts".format(overlay)):
            self.syspath = "{}/{}".format(self.overlays, self.overlay)
        else:
            raise IOError('Device tree overlay source {}.dts does not exist.'.format(self.overlay))

        # if it does not exists create overlay directory
        if not os.path.isdir(self.syspath):
            os.system("mkdir {}".format(self.syspath))

        if self.status():
            print ('Requested overlay is already loaded.')
            # TODO: issuing a working does not work as expected,
            # since __init__ does not finish properly, __del__ is run
            # and the overlay is removed, which is not the intention
            #raise ResourceWarning('Requested overlay is already loaded.')
        else:
            os.system("dtc -I dts -O dtb -o {0}.dtbo -@ {0}.dts".format(self.overlay))
            # TODO: loading FPGA should be handled by device tree overlay
            os.system('cat /opt/redpitaya/fpga/{}/fpga.bit > /dev/xdevcfg'.format(self.overlay))
            os.system("cat {}.dtbo > {}/dtbo".format(self.overlay, self.syspath))

    def __del__ (self):
        print ('Overlay __del__ was activated.')
        os.system("rmdir {}".format(self.syspath))
        del(self.syspath)
        del(self.overlay)

    def status (self):
        if not os.path.exists(self.syspath):
            return (False)
        else:
            with open('{}/status'.format(self.syspath), 'r') as status_file:
                status_string = status_file.read()
                if   (status_string ==   "applied\n"): return (True)
                elif (status_string == "unapplied\n"): return (False)
                else:                                  return (None)
