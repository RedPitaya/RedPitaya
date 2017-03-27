import os

class overlay (object):
    overlays = "/sys/kernel/config/device-tree/overlays"
    fpgapath = "/opt/redpitaya/fpga"

    def __init__ (self, overlay:str):
        if not isinstance(overlay, str):
            raise TypeError("Bitstream name has to be a string.")

        dtbo = "{}/{}/fpga.dtbo".format(self.fpgapath, overlay)
        bit  = "{}/{}/fpga.bit".format(self.fpgapath, overlay)
        
        if os.path.isfile(dtbo):
            self.overlay = overlay
            self.syspath = "{}/{}".format(self.overlays, self.overlay)
        else:
            raise IOError('Device tree overlay source {} does not exist.'.format(dtbo))

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
            #os.system("dtc -I dts -O dtb -o {0}.dtbo -@ {0}.dts".format(self.overlay))
            # TODO: loading FPGA should be handled by device tree overlay
            os.system("cat {} > /dev/xdevcfg".format(bit))
            os.system("cat {} > {}/dtbo".format(dtbo, self.syspath))

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