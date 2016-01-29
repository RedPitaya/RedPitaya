#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "redpitaya/rp2.h"
#include "gpio.h"

int main (int argc, char **argv) {
    int unsigned period = 1000000; // uS
    int unsigned led;
    rp_handle_uio_t handle;

    // index of blinking LED can be provided as an argument
    if (argc > 1) {
        led = atoi(argv[1]);
    // otherwise LED 0 will blink
    } else {
        led = 0;
    }
    printf("Blinking LED[%u]\n", led);

    // Initialization of API
    if (rp_GpioOpen("/dev/uio3", &handle) != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    rp_GpioSetEnable(&handle, 0xff);
    int unsigned retries = 1000;
    while (retries--){
        rp_GpioSetState(&handle, 1<<led);
        usleep(period/2);
        rp_GpioSetState(&handle, 0);
        usleep(period/2);
    }

    // Releasing resources
    rp_GpioClose(&handle);

    return EXIT_SUCCESS;
}
