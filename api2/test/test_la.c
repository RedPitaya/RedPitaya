#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "redpitaya/rp2.h"
#include "la_acq.h"

int main (int argc, char **argv) {
    rp_handle_uio_t handle;
//    int status;
    int unsigned length;

    if (argc > 1) {
        length = atoi(argv[1]);
    } else {
        length = 256;
    }
    printf("Length = %u\n", length);

    // Initialization of API
    if (rp_LaAcqOpen("/dev/uio12", &handle) != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    rp_la_cfg_regset_t cfg;
    cfg.pre=0;
    cfg.pst=length;
    rp_LaAcqSetCntConfig (&handle, cfg);

    rp_LaAcqSetConfig    (&handle, 0x2);
    rp_LaAcqGlobalTrigSet(&handle, 0x0);
    rp_LaAcqRunAcq       (&handle);

    bool tmp;
    do {
        rp_LaAcqAcqIsStopped (&handle, &tmp);
    } while (!tmp);

    // Releasing resources
    rp_LaAcqClose(&handle);

    return EXIT_SUCCESS;
}
