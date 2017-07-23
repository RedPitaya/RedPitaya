/**
 * @brief Red Pitaya library API interface implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#define ioread32(p) (*(volatile uint32_t *)(p))

int main (int argc, char **argv) {
    int fd;
    volatile void *regset;
    volatile void *regset1;
    size_t length;
    off_t offset;
    off_t page_size = sysconf(_SC_PAGESIZE);

    printf("DEBUG: page size is 0x%08lx\n", page_size);

    // try opening the device
    fd = open("/dev/uio/ps2pl", O_RDWR);
    if (fd == -1) {
        printf("ERROR: opening UIO file descriptor failed\n");
        return -1;
    }

    // get regset pointer
    length = 0x1000; // 12 bit page size
    offset = 1 * page_size;
    regset1 = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
    //printf ("1:0x%08x\n\r", (uint32_t) regset);
    if (regset1 == MAP_FAILED) {
        printf("ERROR: mmap failed offset=0x%08lx '%s'\n", offset, strerror(errno));
        //printf ("2:0x%08x\n\r", (uint32_t) regset);
        return (-1);
    } else {
        printf("SUCESS: mmap sucess offset=0x%08lx\n", offset);
    }

    // get regset pointer
    length = 0x1000; // 12 bit page size
    offset = 0x0;
    regset = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
    //printf ("1:0x%08x\n\r", (uint32_t) regset);
    if (regset == MAP_FAILED) {
        printf("ERROR: mmap failed offset=0x%08lx '%s'\n", offset, strerror(errno));
        //printf ("2:0x%08x\n\r", (uint32_t) regset);
        return (-1);
    } else {
        printf("SUCESS: mmap sucess offset=0x%08lx\n", offset);
    }

    uint64_t dna;
    dna = ((uint64_t) ioread32(regset + 0x14) << 32)
        | ((uint64_t) ioread32(regset + 0x10) <<  0);
    printf("DNA=0x%" PRIx64 "\n", dna);

    // release regset pointer
    if (munmap((void *) regset, length) == -1) {
        printf("ERROR: munmap failed\n");
        return (-1);
    }

    // release regset pointer
    if (munmap((void *) regset1, length) == -1) {
        printf("ERROR: munmap failed\n");
        return (-1);
    }

    // close device
    if (close (fd) == -1) {
        printf("ERROR: closing UIO file descriptor failed\n");
        return (-1);
    }
    return (0);
}
