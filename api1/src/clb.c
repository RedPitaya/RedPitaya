#include <math.h>

#include "redpitaya/fixp.h"
#include "redpitaya/clb.h"

int rp_clb_init (rp_clb_t *handle) {
    const char path [] = "/dev/uio/clb";
    
    int status = rp_uio_init (&handle->uio, path);
    if (status) {
        return (-1);
    }
    handle->regset = (rp_clb_regset_t *) handle->uio.map[0].mem;

    // TODO: generalize this, the init function should probably accept types
    // TODO: ADC and DAC handles should be allocated first
    handle->num_dac = 2;
    handle->num_adc = 2;
    int bits_dac = 14;
    int bits_adc = 16;

    // DAC configuration
    for (int unsigned i=0; i<handle->num_dac; i++) {
        const fixp_t mul_t = {.s = 1, .m = 1, .f = bits_dac-2};
        const fixp_t sum_t = {.s = 1, .m = 0, .f = bits_dac-1};
        rp_lin_init(&handle->dac[i], &handle->regset->dac[i], mul_t, sum_t);
    }
    // ADC configuration
    for (int unsigned i=0; i<handle->num_adc; i++) {
        const fixp_t mul_t = {.s = 1, .m = 1, .f = bits_adc-2};
        const fixp_t sum_t = {.s = 1, .m = 0, .f = bits_adc-1};
        rp_lin_init(&handle->adc[i], &handle->regset->adc[i], mul_t, sum_t);
    }

    return(0);
}

int rp_clb_release (rp_clb_t *handle) {
    int status = rp_uio_release (&(handle->uio));
    if (status) {
        return (-1);
    }

    return(0);
}

int rp_clb_default(rp_clb_t *handle) {
    for (int unsigned i=0; i<handle->num_dac; i++) {
        rp_lin_default(&handle->dac[i]);
    }
    for (int unsigned i=0; i<handle->num_adc; i++) {
        rp_lin_default(&handle->adc[i]);
    }
    return(0);
}

void rp_clb_print(rp_clb_t *handle) {
    for (int unsigned i=0; i<handle->num_dac; i++) {
        rp_lin_print(&handle->dac[i]);
    }
    for (int unsigned i=0; i<handle->num_adc; i++) {
        rp_lin_print(&handle->adc[i]);
    }
}

//    def eeprom_read(self, eeprom_offset = _eeprom_offset_user):
//        # open EEPROM device
//        try:
//            eeprom_file = open(self._eeprom_device, 'rb')
//        except OSError as e:
//            raise IOError(e.errno, "Opening {}: {}".format(uio, e.strerror))
//
//        # seek to calibration data
//        try:
//            eeprom_file.seek(eeprom_offset)
//        except IOError as e:
//            raise IOError(e.errno, "Seek {}: {}".format(uio, e.strerror))
//
//        # read calibration data into a buffer
//        try:
//            buffer = eeprom_file.read(sizeof(self._eeprom_t))
//        except IOError as e:
//            raise IOError(e.errno, "Read {}: {}".format(uio, e.strerror))
//
//        # close EEPROM device
//        try:
//            eeprom_file.close()
//        except IOError as e:
//            raise IOError(e.errno, "Close {}: {}".format(uio, e.strerror))
//
//        # map buffer onto structure
//        eeprom_struct = self._eeprom_t.from_buffer_copy(buffer)
//        return eeprom_struct

static float FullScaleToVoltage(int32_t cnt) {
        if (cnt == 0)
            return(1.0);
        else
            return((float) cnt * 100.0 / ldexpf(1, 32));
}

//static int32_t FullScaleFromVoltage(float voltage) {
//        return ((int32_t) (voltage / 100.0 * ldexpf(1, 32)));
//}

int rp_clb_eeprom_parse(rp_clb_t *handle, rp_clb_eeprom_t eeprom_struct, rp_clb_float_t *clb_struct) {
    // convert EEPROM values into local float values
    for (int unsigned i=0; i<handle->num_adc; i++) {
        if (eeprom_struct.magic == CLB_MAGIC) {
            clb_struct->adc[i].lo.gain = FullScaleToVoltage(eeprom_struct.adc_lo_gain  [i]) / 20.0;
            clb_struct->adc[i].hi.gain = FullScaleToVoltage(eeprom_struct.adc_hi_gain  [i]);
        } else {
            clb_struct->adc[i].lo.gain = FullScaleToVoltage(eeprom_struct.adc_hi_gain  [i]);
            clb_struct->adc[i].hi.gain = FullScaleToVoltage(eeprom_struct.adc_lo_gain  [i]) / 20.0;
        }
        clb_struct->adc[i].lo.offset   =                    eeprom_struct.adc_lo_offset[i]  / ((1<<13)-1);
        if (eeprom_struct.magic == CLB_MAGIC) {
            clb_struct->adc[i].hi.offset =                  eeprom_struct.adc_hi_offset[i]  / ((1<<13)-1) * 20.0;
        } else {
            clb_struct->adc[i].hi.offset = clb_struct->adc[i].lo.offset;
        }
    }
    for (int unsigned i=0; i<handle->num_dac; i++) {
        clb_struct->dac[i].gain      = FullScaleToVoltage (eeprom_struct.dac_gain     [i]);
        clb_struct->dac[i].offset    =                     eeprom_struct.dac_offset   [i]  / ((1<<13)-1);
    }
    // missing magic number means a deprecated EEPROM structure was still not updated
    if (eeprom_struct.magic != CLB_MAGIC) {
        for (int unsigned i=0; i<handle->num_adc; i++) {
            clb_struct->adc[i].hi.offset = clb_struct->adc[i].lo.offset;
        }
    }

    // TODO: it probably is possible to get an error
    return (0);
}

//    def calib_show (self, clb_struct):
//        for ch in self.channels_adc:
//            print('adc[{}].lo.gain   = {}'.format(ch, clb_struct.adc[ch].lo.gain))
//            print('adc[{}].hi.gain   = {}'.format(ch, clb_struct.adc[ch].hi.gain))
//            print('adc[{}].lo.offset = {}'.format(ch, clb_struct.adc[ch].lo.offset))
//            print('adc[{}].hi.offset = {}'.format(ch, clb_struct.adc[ch].hi.offset))
//        for ch in self.channels_dac:
//            print('dac[{}].gain      = {}'.format(ch, clb_struct.dac[ch].gain))
//            print('dac[{}].offset    = {}'.format(ch, clb_struct.dac[ch].offset))
//
//int rp_clb_apply (self, clb_struct, adc_range = ['lo', 'lo']):
//    for ch in self.channels_adc:
//        if   (adc_range[ch] == 'lo'):
//            self.adc[ch].gain   = clb_struct.adc[ch].lo.gain
//            self.adc[ch].offset = clb_struct.adc[ch].lo.offset
//        elif (adc_range[ch] == 'hi'):
//            self.adc[ch].gain   = clb_struct.adc[ch].hi.gain
//            self.adc[ch].offset = clb_struct.adc[ch].hi.offset
//        else:
//            raise ValueError("ADC range can be one of ['lo', 'hi'].")
//    for ch in self.channels_dac:
//        self.dac[ch].gain   = clb_struct.dac[ch].gain
//        self.dac[ch].offset = clb_struct.dac[ch].offset
