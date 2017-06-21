#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "uio.h"
#include "evn.h"
#include "gen.h"

int rp_gen_init (rp_gen_t *handle, const int unsigned index) {
    static char path_gen [] = "/dev/uio/gen";
    size_t len = strlen(path_gen)+1+1;
    char path[len];

    // initialize constants
    // sampling frequency
    handle->FS = 125000000.0;
    // linear addition multiplication register width
    handle->DW  = 14;
    handle->DWM = 14;
    handle->DWS = 14;
    // buffer counter ranges
    handle->buffer_size = fixp_num(handle->CWM);
    // burst counter parameters
    handle->CWR = 14;
    handle->CWL = 32;
    handle->CWN = 16;

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

    return(0);
}

int rp_gen_release (rp_gen_t *handle) {
    // disable output
    rp_gen_set_enable(handle, false);
    // reset hardware
    rp_gen_reset(handle);

    int status = rp_uio_release (&(handle->uio));
    if (status) {
        return (-1);
    }

    return(0);
}

// event function wrappers
void     rp_gen_reset         (rp_gen_t *handle)                  {       rp_evn_reset         (&handle->regset->evn) ;}
void     rp_gen_start         (rp_gen_t *handle)                  {       rp_evn_start         (&handle->regset->evn) ;}
void     rp_gen_stop          (rp_gen_t *handle)                  {       rp_evn_stop          (&handle->regset->evn) ;}
void     rp_gen_trigger       (rp_gen_t *handle)                  {       rp_evn_trigger       (&handle->regset->evn) ;}
bool     rp_gen_status_run    (rp_gen_t *handle)                  {return(rp_evn_status_run    (&handle->regset->evn));}
bool     rp_gen_status_trigger(rp_gen_t *handle)                  {return(rp_evn_status_trigger(&handle->regset->evn));}
uint32_t rp_gen_get_sync_src  (rp_gen_t *handle)                  {return(rp_evn_get_sync_src  (&handle->regset->evn));}
void     rp_gen_set_sync_src  (rp_gen_t *handle, uint32_t value)  {       rp_evn_set_sync_src  (&handle->regset->evn, value);}
uint32_t rp_gen_get_trig_src  (rp_gen_t *handle)                  {return(rp_evn_get_trig_src  (&handle->regset->evn));}
void     rp_gen_set_trig_src  (rp_gen_t *handle, uint32_t value)  {       rp_evn_set_trig_src  (&handle->regset->evn, value);}


int rp_gen_set_enable(rp_gen_t *handle, bool value) {
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
