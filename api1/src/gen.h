#ifndef __RP_GEN_H__
#define __RP_GEN_H__

#include <stdint.h>

#include "uio.h"


// sampling frequency
FS = 125000000.0
/ linear addition multiplication register width
W  = 14  #: data width - streaming sample
DWM = 14  #: data width - linear gain multiplier
DWS = 14  #: data width - linear offset summand
// fixed point range
_DWr  = (1 << (DW -1)) - 1
_DWMr = (1 << (DWM-2))
_DWSr = (1 << (DWS-1)) - 1
// buffer parameters (fixed point number uM.F)
CWM = 14  #: counter width - magnitude (fixed point integer)
CWF = 16  #: counter width - fraction  (fixed point fraction)
CW  = CWM + CWF
// buffer counter ranges
_CWMr = 2**CWM
_CWFr = 2**CWF
buffer_size = 2**CWM #: table size
// burst counter parameters
CWR = 14  #: counter width - burst data repetition
CWL = 32  #: counter width - burst period length
CWN = 16  #: counter width - burst period number
_CWRr = 2**CWR
_CWLr = 2**CWL
_CWNr = 2**CWN

// logaritmic scale from 0.116Hz to 62.5Mhz
_f_min = FS / 2**CW
_f_max = FS / 2
_f_one = FS / 2**CWM

typedef volatile struct {
    rp_evn_regset_t     evn;
    uint32_t            rsv0;     // reserved
    uint32_t            cfg_bmd;  // mode [1:0] = [inf, ben]
    rp_asg_per_regset_t per;
    rp_asg_bst_regset_t bst;
    uint32_t            rsv1[2];  // reserved
    rp_gen_out_regset_t out;
} rp_gen_regset_t;

typedef struct {
    rp_uio_t uio;
    rp_gen_regset_t *regset;
    int16_t          table[];
} rp_hwid_t;


int rp_gen_init    (rp_gen_t *handle);
int rp_gen_release (rp_gen_t *handle);
    
uint32_t rp_hwid_get_hwid  (rp_hwid_t *handle);
uint32_t rp_hwid_get_efuse (rp_hwid_t *handle);
uint64_t rp_hwid_get_dna   (rp_hwid_t *handle);

#endif

