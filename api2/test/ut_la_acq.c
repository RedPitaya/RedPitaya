#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <stdlib.h>
#include <stdint.h>

#include "rp2.h"
#include "la_acq.h"

int main (int argc, char **argv) {

    rp_handle_uio_t handle;

    // Open logic analyzer device
    if(rp_LaGenOpen("/dev/dummy", &handle) != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    rp_LaGenReset(&handle);
    rp_LaGenRunAcq(&handle);
    rp_LaGenStopAcq(&handle);
    rp_LaGenTriggerAcq(&handle);

    bool status;
    rp_LaGenAcqIsStopped(&handle, &status);

    rp_la_cfg_regset_t cfgw, cfgr;
    cfgw.acq=1234;
    cfgw.trg=1235;
    cfgw.pre=1236;
    cfgw.pst=1237;
    rp_LaGenSetConfig(&handle,cfgw);
    rp_LaGenGetConfig(&handle,&cfgr);
    if(memcmp((char*)&cfgw, (char*)&cfgr, sizeof(rp_la_cfg_regset_t) )!=0){
    	fprintf(stderr, "not the same\n");
    }

    // others..

    // Close log. anal. device
    if(rp_LaGenClose(&handle) != RP_OK){
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
