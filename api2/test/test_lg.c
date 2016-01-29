#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "redpitaya/rp2.h"
#include "generate.h"

int main (int argc, char **argv) {
    rp_handle_uio_t handle;

    // Initialization of API
    if (rp_GenOpen("/dev/uio11", &handle) != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    // Releasing resources
    rp_GenClose(&handle);

    return EXIT_SUCCESS;
}
