#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "redpitaya/rp2.h"
#include "generate.h"

int main (int argc, char **argv) {
    rp_handle_uio_t handle;
    int status;

    // Initialization of API
    if (rp_GenOpen("/dev/uio11", &handle) != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    int unsigned length = 256;
    uint32_t stp = 0x00010000;
    uint32_t off = 0x00000000;

    rp_GenSetWaveformUpCountSeq(&handle, length);

    status = rp_GenSetStepOffset(&handle, stp, off);
    if (status != RP_OK) {
        fprintf(stderr, "Error: failed to write step and offset!\n");
        return EXIT_FAILURE;
    }

    rp_GenOutputEnable(&handle, 0xffff);

    rp_GenSetMode               (&handle, RP_GEN_MODE_BURST);
    rp_GenSetBurstModeDataLen   (&handle, length);
    rp_GenSetBurstModePeriodLen (&handle, length);

    rp_GenSetBurstModeRepetitions (&handle, RP_GEN_REP_INF);

    rp_GenGlobalTrigSet(&handle, RP_TRG_DGEN_SWE_MASK);
    rp_GenTrigger(&handle);

    // Releasing resources
    rp_GenClose(&handle);

    return EXIT_SUCCESS;
}
