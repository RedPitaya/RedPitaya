#include <stdio.h>
#include <stdlib.h>

#include "rp.h"

int main (int argc, char **argv) {
    rp_pinState_t state;

    // Initialization of API
    if (rp_Init() != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    // configure DIO[0:7]_N to inputs
    for (int i=0; i<8; i++) {
        rp_DpinSetDirection (i+RP_DIO0_N, RP_IN);
    }

    // transfer each input state to the corresponding LED state
    while (1) {
        for (int i=0; i<8; i++) {
            rp_DpinGetState (i+RP_DIO0_N, &state);
            rp_DpinSetState (i+RP_LED0, state);
        }
    }

    // Releasing resources
    rp_Release();

    return EXIT_SUCCESS;
}
