/** 
 * $Id: fpga_awg.c 881 2013-12-16 05:37:34Z rp_jmenart $
 * 
 * @brief Red Pitaya Arbitrary Waveform Generator (AWG) FPGA controller.
 *
 * @Author Jure Menart <juremenart@gmail.com>
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
#include <math.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "fpga_awg.h"

/** 
 * GENERAL DESCRIPTION:
 *
 * This module initializes and provides for other SW modules the access to the 
 * FPGA AWG module. The AWG memory space is divided to three parts:
 *  - registers
 *  - signal buffer 1
 *  - signal buffer 2
 *
 * This module maps physical address of the AWG core to the logical address, 
 * which can be used in the GNU/Linux user-space. To achieve this AWG_BASE_ADDR
 * from CPU memory space is translated automatically to logical address with the
 * function mmap(). After all the initialization is done, other modules can use
 * fpga_awg to control signal outputs (as an example please see generate.c).
 * Before this module is used external SW module must call fpga_awg_init().
 * When this module is no longer needed fpga_awg_exit() should be called.
 *
 * FPGA AWG core state machine operates depending how it is configured. Basic 
 * principle is that after a trigger it reads output signal buffer (for each 
 * channel separately) with configured offset, step (for each clock cycle) and 
 * length (until it stops or wrap readout) - the read-out value is output over 
 * DAC to RF output. For more detailed operation see the FPGA AWG register 
 * description.
 */

/* Internal structures (registers and 2 times signal buffers) */
/** The FPGA register structure (defined in fpga_awg.h) */
awg_reg_t *g_awg_reg     = NULL;
/** The FPGA signal buffer 1.
 * The length of buffer is defined in the FPGA and is AWG_SIG_LEN. Each sample in
 * the buffer is 14-bit signed integer.
  */
uint32_t  *g_awg_cha_mem = NULL;
/** The FPGA signal buffer 2
 * The length of buffer is defined in the FPGA and is AWG_SIG_LEN. Each sample in
 * the buffer is 14-bit signed integer.
  */
uint32_t  *g_awg_chb_mem = NULL;

/** The memory file descriptor used to mmap() the FPGA space */
int g_awg_fd = -1;

/* Constants */
/** DAC frequency (125 Mspmpls (non-decimated)) */
const double c_awg_smpl_freq = 125e6;

/**
 * Internal function used to clean up memory.
 *
 * This function un-maps FPGA register and signal buffers, closes memory file
 * descriptor and cleans all memory allocated by this module.
 *
 * @retval 0 Success
 * @retval -1 Error happened during cleanup.
 */
int __awg_cleanup_mem(void)
{
    /* If registry structure is NULL we do not need to un-map and clean up */
    if(g_awg_reg) {
        if(munmap(g_awg_reg, AWG_BASE_SIZE) < 0) {
            fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
            return -1;
        }
        g_awg_reg = NULL;
        if(g_awg_cha_mem)
            g_awg_cha_mem = NULL;
        if(g_awg_chb_mem)
            g_awg_chb_mem = NULL;
    }
    if(g_awg_fd >= 0) {
        close(g_awg_fd);
        g_awg_fd = -1;
    }
    return 0;
}

/**
 * @brief Maps FPGA memory space and prepares register and buffer variables.
 * 
 * This function opens memory device (/dev/mem) and maps physical memory address
 * AWG_BASE_ADDR (of length AWG_BASE_SIZE) to logical addresses. It initializes
 * the pointers g_awg_reg, g_awg_cha_mem, g_awg_chb_mem to point to FPGA AWG.
 * If function failes FPGA variables must not be used.
 *
 * @retval 0  Success
 * @retval -1 Failure, error is printed to standard error output.
 */
int fpga_awg_init(void)
{
    /* Page variables used to calculate correct mapping addresses */
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    /* If module was already initialized, clean all internals */
    if(__awg_cleanup_mem() < 0)
        return -1;

    /* Open /dev/mem to access directly system memory */
    g_awg_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(g_awg_fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    /* Calculate correct page address and offset from AWG_BASE_ADDR and
     * AWG_BASE_SIZE 
     */
    page_addr = AWG_BASE_ADDR & (~(page_size-1));
    page_off  = AWG_BASE_ADDR - page_addr;

    /* Map FPGA memory space to page_ptr. */
    page_ptr = mmap(NULL, AWG_BASE_SIZE, PROT_READ | PROT_WRITE,
                          MAP_SHARED, g_awg_fd, page_addr);
    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
         __awg_cleanup_mem();
        return -1;
    }

    /* Set FPGA AWG module pointers to correct values. */
    g_awg_reg = page_ptr + page_off;
    g_awg_cha_mem = (uint32_t *)g_awg_reg + 
        (AWG_CHA_OFFSET / sizeof(uint32_t));
    g_awg_chb_mem = (uint32_t *)g_awg_reg + 
        (AWG_CHB_OFFSET / sizeof(uint32_t));

    return 0;
}

/**
 * @brief Cleans up FPGA AWG module internals.
 * 
 * This function closes the memory file descriptor, unmap the FPGA memory space
 * and cleans also all other internal things from FPGA AWG module.
 * @retval 0 Sucess
 * @retval -1 Failure
 */
int fpga_awg_exit(void)
{
    return __awg_cleanup_mem();
}
