/**
 * @brief Red Pitaya PID FPGA controller.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
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

#include "fpga_pid.h"

/** 
 * GENERAL DESCRIPTION:
 *
 * This module initializes and provides for other SW modules the access to the 
 * FPGA PID module.
 *
 * This module maps physical address of the PID core to the logical address,
 * which can be used in the GNU/Linux user-space. To achieve this PID_BASE_ADDR
 * from CPU memory space is translated automatically to logical address with the
 * function mmap().
 * Before this module is used external SW module must call fpga_pid_init().
 * When this module is no longer needed fpga_pid_exit() should be called.
 */

/** The FPGA register structure (defined in fpga_pid.h) */
pid_reg_t *g_pid_reg     = NULL;

/** The memory file descriptor used to mmap() the FPGA space */
int g_pid_fd = -1;


/*----------------------------------------------------------------------------*/
/**
 * @brief Internal function used to clean up memory.
 *
 * This function un-maps FPGA registers, closes memory file
 * descriptor and cleans all memory allocated by this module.
 *
 * @retval 0 Success
 * @retval -1 Failure, error is printed to standard error output.
 */
int __pid_cleanup_mem(void)
{
    /* If registry structure is NULL we do not need to un-map and clean up */
    if(g_pid_reg) {
        if(munmap(g_pid_reg, PID_BASE_SIZE) < 0) {
            fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
            return -1;
        }
        g_pid_reg = NULL;
    }

    if(g_pid_fd >= 0) {
        close(g_pid_fd);
        g_pid_fd = -1;
    }
    return 0;
}

/** Reset all PIDs */
void reset_pids(void)
{
    if (g_pid_reg) {

        int i;
        for (i = 0; i < NUM_OF_PIDS; i++) {
            g_pid_reg->pid[i].setpoint = 0;
            g_pid_reg->pid[i].kp = 0;
            g_pid_reg->pid[i].ki = 0;
            g_pid_reg->pid[i].kd = 0;
        }

        g_pid_reg->configuration = 0xf;
        g_pid_reg->configuration = 0;
    }
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Maps FPGA memory space and prepares register variables.
 * 
 * This function opens memory device (/dev/mem) and maps physical memory address
 * PID_BASE_ADDR (of length PID_BASE_SIZE) to logical addresses. It initializes
 * the pointer g_pid_reg to point to FPGA PID.
 * If function fails FPGA variables must not be used.
 *
 * @retval 0  Success
 * @retval -1 Failure, error is printed to standard error output.
 */
int fpga_pid_init(void)
{
    /* Page variables used to calculate correct mapping addresses */
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    /* If module was already initialized, clean all internals */
    if(__pid_cleanup_mem() < 0)
        return -1;

    /* Open /dev/mem to access directly system memory */
    g_pid_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(g_pid_fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    /* Calculate correct page address and offset from PID_BASE_ADDR and
     * PID_BASE_SIZE
     */
    page_addr = PID_BASE_ADDR & (~(page_size-1));
    page_off  = PID_BASE_ADDR - page_addr;

    /* Map FPGA memory space to page_ptr. */
    page_ptr = mmap(NULL, PID_BASE_SIZE, PROT_READ | PROT_WRITE,
                          MAP_SHARED, g_pid_fd, page_addr);
    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
         __pid_cleanup_mem();
        return -1;
    }

    /* Set FPGA PID module pointers to correct values. */
    g_pid_reg = page_ptr + page_off;

    /* Reset all controllers */
    reset_pids();

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Cleans up FPGA PID module internals.
 * 
 * This function closes the memory file descriptor, unmaps the FPGA memory space
 * and cleans also all other internal things from FPGA PID module.
 * @retval 0 Success
 * @retval -1 Failure
 */
int fpga_pid_exit(void)
{
    /* Reset all controllers */
    reset_pids();

    return __pid_cleanup_mem();
}
