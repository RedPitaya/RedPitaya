#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "uio.h"
#include "evn.h"
#include "gen.h"
#include "asg_per.h"
#include "asg_bst.h"
#include "gen_out.h"

int rp_gen_init (rp_gen_t *handle, const int unsigned index) {
    static char path_gen [] = "/dev/uio/gen";
    size_t len = strlen(path_gen)+1+1;
    char path[len];

    // initialize constants
    handle->FS = 125000000.0;  // sampling frequency
    handle->buffer_size = 1<<14;
    handle->dat_t = (fixp_t) {.s = 1, .m = 0, .f = DW-1};

    // a single character is reserved for the index
    // so indexes above 9 are an error
    if (index>9) {
        fprintf(stderr, "GEN: failed device index decimal representation is limited to 1 digit.\n");
        return (-1);
    }
    snprintf(path, len, "%s%u", path_gen, index);
    
    int status = rp_uio_init (&handle->uio, path);
    if (status) {
        return (-1);
    }
    // map regset
    handle->regset = (rp_gen_regset_t *) handle->uio.map[0].mem;
    // map table
    handle->table = (int16_t *) handle->uio.map[0].mem;

    // events
    rp_evn_init(&handle->evn, &handle->regset->evn);
    // continuous/periodic mode
    const fixp_t cnt_t = {.s = 0, .m = CWM, .f = CWF};
    rp_asg_per_init(&handle->per, &handle->regset->per, handle->FS, handle->buffer_size, cnt_t);
    // burst mode
    const fixp_t bdr_t = {.s = 0, .m = CWR, .f = 0};
    const fixp_t bpl_t = {.s = 0, .m = CWL, .f = 0};
    const fixp_t bpn_t = {.s = 0, .m = CWN, .f = 0};
    rp_asg_bst_init(&handle->bst, &handle->regset->bst, handle->FS, handle->buffer_size, bdr_t, bpl_t, bpn_t);
    // output configuration
    const fixp_t mul_t = {.s = 1, .m = 1, .f = DWM-2};
    const fixp_t sum_t = {.s = 1, .m = 0, .f = DWS-1};
    rp_gen_out_init(&handle->out, &handle->regset->out, mul_t, sum_t);

    return(0);
}

int rp_gen_release (rp_gen_t *handle) {
    // disable output
    rp_gen_out_set_enable(&handle->out, false);
    // reset hardware
    rp_evn_reset(&handle->evn);

    int status = rp_uio_release (&handle->uio);
    if (status) {
        return (-1);
    }

    return(0);
}

//    @property
//    def waveform (self):
//        """Waveworm array containing normalized values in the range [-1,1].
//
//        Array can be up to `buffer_size` samples in length.
//        """
//        siz = self.table_size
//        # TODO: nparray
//        return [self.table[i] / self._DWr for i in range(siz)]
//
//    @waveform.setter
//    def waveform (self, value):
//        siz = len(value)
//        if (siz <= self.buffer_size):
//            for i in range(siz):
//                # TODO add saturation
//                self.table[i] = int(value[i] * self._DWr)
//            self.table_size = siz
//        else:
//            raise ValueError("Waveform table size should not excede buffer size. buffer_size = {}".format(self.buffer_size))
//
//    class modes(Enum):
//        CONTINUOUS = 0x0
//        FINITE     = 0x1
//        INFINITE   = 0x3
//
//    @property
//    def mode (self) -> str:
//        """Generator mode:
//
//        * 'CONTINUOUS' - non burst mode for continuous/periodic signals
//        * 'FINITE'     - finite    length bursts
//        * 'INFINITE'   - inifinite length bursts
//        """
//        return (self.modes(self.regset.cfg_bmd))
//
//    @mode.setter
//    def mode (self, value: str):
//        if isinstance(value, str):
//            self.regset.cfg_bmd = self.modes[value].value
//        else:
//            raise ValueError("Generator supports modes ['CONTINUOUS', 'FINITE', 'INFINITE'].")
