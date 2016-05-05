/**
 * @brief Red Pitaya FPGA Interface for its sub-modules.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "fpga.h"


/*----------------------------------------------------------------------------*/
int fpga_init(void)
{
    int state = 0;
    int ret;

    // init the system XADC FPGA module
    ret = fpga_sys_xadc_init();
    if (ret) {
        state = ret;
    }

    // init the House-keeping FPGA sub-module
    ret = fpga_hk_init();
    if (ret) {
        state = ret;
    }

    // init the RadioBox FPGA sub-module
    ret = fpga_rb_init();
    if (ret) {
        state = ret;
    }

    return state;
}

/*----------------------------------------------------------------------------*/
int fpga_exit(void)
{
    // exit access to the RadioBox sub-module
    fpga_rb_exit();

    // exit access to the House-keeping sub-module
    fpga_hk_exit();

    // exit access to the system XADC FPGA module
    fpga_sys_xadc_exit();

    return 0;
}


int fpga_mmap_area(int* fd, void** mem, long base_addr, long base_size)
{
    const long page_size = sysconf(_SC_PAGESIZE);

    *fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (*fd < 0) {
        fprintf(stderr, "ERROR - fpga_mmap_area - open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }
    long page_addr = base_addr & (~(page_size-1));
    long page_offs = base_addr - page_addr;

    void* page_ptr = mmap(NULL, page_offs + base_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, *fd, page_addr);
    if (page_ptr == MAP_FAILED) {
        fprintf(stderr, "ERROR - fpga_mmap_area - mmap() failed: %s\n", strerror(errno));
        return -2;
    }

    *mem = page_ptr + page_offs;
    return 0;
}

int fpga_munmap_area(int* fd, void** mem, long base_addr, long base_size)
{
    if (*mem) {
        const long page_size = sysconf(_SC_PAGESIZE);

        long page_addr = base_addr & (~(page_size-1));
        long page_offs = base_addr - page_addr;

        if (munmap(*mem, page_offs + base_size) < 0) {
            fprintf(stderr, "ERROR - fpga_munmap_area - munmap() failed: %s\n", strerror(errno));
            return -1;
        }
        *mem = NULL;
    }

    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
    return 0;
}
