#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include "rpdma.h"

#include "redpitaya/rp2.h"
#include "la_acq.h"
#include "rp_dma.h"

#define SIZE 0x100

int main (int argc, char **argv) {
    rp_handle_uio_t handle;
    int status;
    int unsigned length;

    if (argc > 1) {
        length = atoi(argv[1]);
    } else {
        length = 64;
    }
    printf("Length = %u\n", length);

    // Initialization of API
    status = rp_LaAcqOpen("/dev/uio/la", &handle);
    if (status != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    rp_SetSgmntC (&handle, 4);
    rp_SetSgmntS (&handle, SIZE*2);
    handle.dma_size = 4*SIZE*2;

    int16_t * map=NULL;
    map = (int16_t *) mmap(NULL, handle.dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle.dma_fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        return -1;
    }

    // configure acquire
    rp_la_cfg_regset_t cfg;
    cfg.pre=0;
    cfg.pst=length;
    rp_LaAcqSetCntConfig (&handle, cfg);
    rp_LaAcqSetConfig    (&handle, 0x2);
    rp_LaAcqGlobalTrigSet(&handle, 0x0);

    // test loop
    for (int n=0; n<6; n++) {
        printf("Repeat: %d\n", n);

        // clear data
        for (int i=0; i<handle.dma_size/sizeof(*map); i++) {
            map[i]=0;
        }

        // run acquire and wait for it to stop
        rp_LaAcqRunAcq       (&handle);
        rp_LaAcqBlockingRead (&handle);

        // printout data
        for (int i=0; i<handle.dma_size/sizeof(*map); i++) {
            if ((i%32)==0 ) printf("@%08x: ", i * sizeof(*map));
            printf("%04x", map[i]);
            if ((i%32)==31) printf("\n");
        }
    }

    // release memory
    status = munmap (map, sizeof(*map));

    // Releasing resources
    rp_LaAcqClose(&handle);

    return EXIT_SUCCESS;
}
