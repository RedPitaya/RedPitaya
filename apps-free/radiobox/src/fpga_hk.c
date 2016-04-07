/**
 * @brief Red Pitaya FPGA Interface for the House-keeping sub-module.
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
#include <string.h>
#include <errno.h>

#include "fpga.h"


/** @brief The House-keeping memory file descriptor used to mmap() the FPGA space. */
extern int                  g_fpga_hk_mem_fd;

/** @brief The House-keeping memory layout of the FPGA registers. */
extern fpga_hk_reg_mem_t*   g_fpga_hk_reg_mem;


/*----------------------------------------------------------------------------*/
int fpga_hk_init(void)
{
    /* make sure all previous data is vanished */
    fpga_hk_exit();

    /* init the HouseKeeping FPGA sub-module access */
    if (fpga_mmap_area(&g_fpga_hk_mem_fd, (void**) &g_fpga_hk_reg_mem, FPGA_HK_BASE_ADDR, FPGA_HK_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_hk_init: g_fpga_hk_reg_mem - mmap() failed: %s\n", strerror(errno));
        fpga_exit();
        return -1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
int fpga_hk_exit(void)
{
    /* unmap the House-keeping sub-module */
    if (fpga_munmap_area(&g_fpga_hk_mem_fd, (void**) &g_fpga_hk_reg_mem, FPGA_HK_BASE_ADDR, FPGA_HK_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_hk_init: g_fpga_hk_reg_mem - munmap() failed: %s\n", strerror(errno));
    }
    return 0;
}


/*----------------------------------------------------------------------------*/
int fpga_hk_setLeds(unsigned char doToggle, unsigned char mask, unsigned char leds)
{
    if (!g_fpga_hk_reg_mem) {
        return -1;
    }

    // setting LEDs
    if (doToggle) {
        g_fpga_hk_reg_mem->leds = (g_fpga_hk_reg_mem->leds          ^  mask       );

    } else {
        g_fpga_hk_reg_mem->leds = (g_fpga_hk_reg_mem->leds & ~mask) | (mask & leds);
    }
    return 0;
}
