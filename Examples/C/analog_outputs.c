/* Set analog voltage on slow analog output */

#include <stdio.h>
#include <stdlib.h>

#include "rp.h"

int main (int argc, char **argv) {
    float value [4];

    // Voltages can be provided as an argument (default is 1V)
    for (int i=0; i<4; i++) {
        if (argc > (1+i)) {
            value [i] = atof(argv[1+i]);
        } else {
            value [i] = 1.0;
        }
        printf("Voltage setting for AO[%i] = %1.1fV\n", i, value [i]);
    }

    // Initialization of API
    if (rp_Init() != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    // Setting a voltage for each ananlog output
    for (int i=0; i<4; i++) {
        int status = rp_AOpinSetValue(i, value[i]);
        if (status != RP_OK) {
            printf("Could not set AO[%i] voltage.\n", i);
        }
    }

    // wait for user input
    getchar();

    // Releasing resources
    rp_Release();

    return EXIT_SUCCESS;
}
