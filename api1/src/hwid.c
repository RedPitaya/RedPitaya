#include "hwid.h"

int rp_hwid_init (rp_hwid_t *handle) {
    char path [] = "/dev/uio/hwid";
    
    int status = rp_uio_init (&handle->uio, path);
    if (status) {
        return (-1);
    }
    handle->regset = (rp_hwid_regset_t *) handle->uio.map[0].mem;

    return(0);
}

int rp_hwid_release (rp_hwid_t *handle) {
    int status = rp_uio_release (&(handle->uio));
    if (status) {
        return (-1);
    }

    return(0);
}
    
uint32_t rp_hwid_get_hwid (rp_hwid_t *handle) {
    return (handle->regset->hwid);
}

uint32_t rp_hwid_get_efuse (rp_hwid_t *handle) {
    return (handle->regset->efuse);
}

uint64_t rp_hwid_get_dna (rp_hwid_t *handle) {
    return ((uint64_t) handle->regset->dna[1] << 32 |
            (uint64_t) handle->regset->dna[0] << 0 );
}

//    @property
//    def gith (self) -> str:
//        """Git hash.
//        
//        A full SHA-1 hash (160 bits, 40 hex characters)
//        for the repository from which the FPGA was built.
//        """
//        return (''.join(["{:08x}".format(self.regset.gith[i]) for i in range(5)]))

