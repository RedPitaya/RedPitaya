#ifndef __RP_HWID_H__
#define __RP_HWID_H__

#include <stdint.h>

#include "uio.h"

typedef volatile struct {
    uint32_t hwid    ;
    uint32_t rsv0    ;  // reserved
    uint32_t efuse   ;
    uint32_t rsv1    ;  // reserved
    uint32_t dna  [2];
    uint32_t rsv3 [2];  // reserved
    uint32_t gith [5];
} rp_hwid_regset_t;

typedef struct {
    rp_uio_t uio;
    rp_hwid_regset_t *regset;
} rp_hwid_t;


int rp_hwid_init    (rp_hwid_t *handle);
int rp_hwid_release (rp_hwid_t *handle);
    
uint32_t rp_hwid_get_hwid  (rp_hwid_t *handle);
uint32_t rp_hwid_get_efuse (rp_hwid_t *handle);
uint64_t rp_hwid_get_dna   (rp_hwid_t *handle);

#endif

