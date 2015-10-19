#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

int main(int argc, char **argv){
    int unsigned period = 1000000; // uS

    // Print error, if rp_Init() function failed
    if(rp_Init() != RP_OK){
        fprintf(stderr, "Rp api init failed!\n");
    }

    // Set rp_dpin_t to led 1
    rp_dpin_t pin = RP_LED1;

    int retries = 1000; //ms
    while(retries--){
        // Setting pin to 1
        rp_DpinSetState(pin, RP_HIGH);
        usleep(period/2);
        rp_DpinSetState(pin, RP_LOW);
        usleep(period/2);
    }

    // Releasing resources
    rp_Release();

    return 0;
}
