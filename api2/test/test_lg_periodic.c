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
    double frequency = 1000;
    double phase = 180;

    rp_GenSetWaveformUpCountSeq(&handle, length);

    status = rp_GenSetFreqPhase(&handle,  frequency,  phase);
    if (status != RP_OK) {
        fprintf(stderr, "Red Pitaya API access failed!\n");
    }
    status = rp_GenGetFreqPhase(&handle, &frequency, &phase);
    if (status != RP_OK) {
        fprintf(stderr, "Red Pitaya API access failed!\n");
    }

    rp_GenOutputEnable(&handle, 0xffff);

    rp_GenGlobalTrigSet(&handle, RP_TRG_DGEN_SWE_MASK);
    rp_GenTrigger(&handle);

    usleep(5*1000*1000);
    fprintf(stderr, "frequency = %f, phase = %f\n", frequency, phase);

    // Releasing resources
    rp_GenClose(&handle);

    return EXIT_SUCCESS;
}
