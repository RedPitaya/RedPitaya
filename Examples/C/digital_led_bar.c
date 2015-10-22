#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

int main (int argc, char **argv) {
    float percent;

    // percentage can be provided as an argument
    if (argc > 1) {
        percent = atof(argv[1]);
    } else {
        percent = 50.0;
    }
    printf("Bar showing %.1f%%\n", percent);

    // Initialization of API
    if (rp_Init() != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    // Turning on leds based on parameter percent
    for (int i=0; i<8; i++) {
        if (percent > (i*(100.0/8))) {
            rp_DpinSetState(i+RP_LED0, RP_HIGH);
        } else {
            rp_DpinSetState(i+RP_LED0, RP_LOW);
        }
    }

    // Releasing resources
    rp_Release();

    return EXIT_SUCCESS;
}
