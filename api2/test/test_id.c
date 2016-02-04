#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "redpitaya/rp2.h"
#include "id.h"

int main (int argc, char **argv) {
    rp_handle_uio_t handle;

    // Initialization of API
    if (rp_IdOpen("/dev/uio0", &handle) != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    uint32_t id;
    rp_IdGetID (&handle, &id);
    printf("ID = 0x%08x\n", id);

    uint32_t efuse;
    rp_IdGetEFUSE (&handle, &efuse);
    printf("EFUSE = 0x%08x\n", efuse);

    uint64_t dna;
    rp_IdGetDNA (&handle, &dna);
    printf("DNA = 0x%" PRIx64 "\n", dna);

    uint32_t gith[5];
    rp_IdGetGITH (&handle, gith);
    printf("GITH = ");
    for (int i=0; i<5; i++) {
        printf("%08x", gith[i]);
    }
    printf("\n");

    // Releasing resources
    rp_IdClose(&handle);

    return EXIT_SUCCESS;
}
