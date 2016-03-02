/**
 * @brief Red Pitaya FPGA Interface for the RadioBox sub-module.
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
//#include <stdlib.h>
#include <string.h>
//#include <math.h>
//#include <pthread.h>
#include <errno.h>
//#include <sys/mman.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>
//#include <fcntl.h>

#include "main.h"
#include "fpga.h"


/** @brief The system GPIO for XADC memory file descriptor used to mmap() the FPGA space. */
extern int                      g_fpga_sys_xadc_mem_fd;
/** @brief The system GPIO for XADC memory layout of the FPGA registers. */
extern fpga_sys_xadc_reg_mem_t* g_fpga_sys_xadc_reg_mem;

#if 0
/** @brief The system GPIO for LEDs memory file descriptor used to mmap() the FPGA space. */
extern int                      g_fpga_sys_gpio_leds_mem_fd;
/** @brief The system GPIO for LEDs memory layout of the FPGA registers. */
extern fpga_sys_gpio_reg_mem_t* g_fpga_sys_gpio_leds_reg_mem;
#endif


/*----------------------------------------------------------------------------*/
int fpga_sys_xadc_init(void)
{
    //fprintf(stderr, "fpga_sys_xadc_init: BEGIN\n");

    /* make sure all previous data is vanished */
    fpga_sys_xadc_exit();

    /* maps the system XADC module */
    if (fpga_mmap_area(&g_fpga_sys_xadc_mem_fd, (void**) &g_fpga_sys_xadc_reg_mem, FPGA_SYS_XADC_BASE_ADDR, FPGA_SYS_XADC_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_sys_xadc_init: g_fpga_sys_xadc_reg_mem - mmap() failed: %s\n", strerror(errno));
        fpga_exit();
        return -1;
    }
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_xadc_reg_mem - having access pointer.\n");

#if 0
    /* maps the system GPIO LEDs module */
    if (fpga_mmap_area(&g_fpga_sys_gpio_leds_mem_fd, (void**) &g_fpga_sys_gpio_leds_reg_mem, FPGA_SYS_GPIO_LEDS_BASE_ADDR, FPGA_SYS_GPIO_LEDS_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - mmap() failed: %s\n", strerror(errno));
        fpga_exit();
        return -1;
    }
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - having access pointer.\n");
#endif

    {
        uint32_t temp = g_fpga_sys_xadc_reg_mem->stat_temp;
        fprintf(stderr, "DEBUG fpga_sys_xadc_init: 1) temp value of XADC register = %4.2f°C.\n", (((float) temp) * 503.975f / 65536.0f) - 273.15);
    }

#if 0
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: before setting registers.\n");
    sync();

    uint32_t val;
    val = g_fpga_sys_gpio_leds_reg_mem->gpio_data;  // GPIO_DATA
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - Read GPIO_DATA\t= 0x%08x.\n", val);

    val = g_fpga_sys_gpio_leds_reg_mem->gpio_tri;  // GPIO_TRI
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - Read GPIO_TRI\t= 0x%08x.\n", val);

    val = g_fpga_sys_gpio_leds_reg_mem->gpio2_data;  // GPIO2_DATA
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - Read GPIO2_DATA\t= 0x%08x.\n", val);

    val = g_fpga_sys_gpio_leds_reg_mem->gpio2_tri;  // GPIO2_TRI
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - Read GPIO2_TRI\t= 0x%08x.\n", val);

    val = g_fpga_sys_gpio_leds_reg_mem->gier;  // GIER
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - Read GIER\t= 0x%08x.\n", val);

    val = g_fpga_sys_gpio_leds_reg_mem->ip_ier;  // IP IER
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - Read IP IER\t= 0x%08x.\n", val);

    val = g_fpga_sys_gpio_leds_reg_mem->ip_isr;  // IP ISR
    //fprintf(stderr, "DEBUG fpga_sys_xadc_init: g_fpga_sys_gpio_leds_reg_mem - Read IP ISR\t= 0x%08x.\n", val);

    uint32_t i;
    g_fpga_sys_gpio_leds_reg_mem->gpio_tri = 0x00000000;  // all bits are output
    for (i = 0; i < 2048; i++) {
        g_fpga_sys_gpio_leds_reg_mem->gpio_data = 0x00000100 | i;
        usleep(1000);
    }
#endif

    /* reset the AXI XADC IP core */
    g_fpga_sys_xadc_reg_mem->srr                = 0x0000000A;                 // taking 16 clocks
    usleep(1000);
    g_fpga_sys_xadc_reg_mem->sysmonrr           = 0x00000001;
    g_fpga_sys_xadc_reg_mem->sysmonrr           = 0x00000000;

    {
        uint32_t temp = g_fpga_sys_xadc_reg_mem->stat_temp;
        fprintf(stderr, "DEBUG fpga_sys_xadc_init: 2) temp value of XADC register = %4.2f°C.\n", (((float) temp) * 503.975f / 65536.0f) - 273.15);
    }

    g_fpga_sys_xadc_reg_mem->gier               = 0x00000000;                 // global interrupt disabled

    /* settings for simultaneous streaming of XADC CH0/8 and CH1/9 data */
    g_fpga_sys_xadc_reg_mem->conf_1             =    0b01 << 12;              // averaging 16 clocks
    g_fpga_sys_xadc_reg_mem->conf_2             = (0b0100 << 12) | (1 << 8);  // sequencer mode: simultaneous sampling mode - disable Vcc_BRAM alarm
    g_fpga_sys_xadc_reg_mem->conf_3             =       7 <<  8;              // clock division value - 125 MHz to 768 kSPS

    g_fpga_sys_xadc_reg_mem->seq0_ch_sel        =       1 << 11;              // Vp_Vn enabled
    g_fpga_sys_xadc_reg_mem->seq1_ch_sel        =      (1 <<  0) | (1 << 1);  // Vaux_P[1]/Vaux_N[1] - Vaux_P[0]/Vaux_N[0] enabled - continuous mode also Vaux_P[9]/Vaux_N[9] - Vaux_P[8]/Vaux_N[8]
    g_fpga_sys_xadc_reg_mem->seq2_ch_avg        = 0;
    g_fpga_sys_xadc_reg_mem->seq3_ch_avg        = (1 <<  9) | (1 << 8) | (1 << 1) | (1 << 0);  // averaging CH9 CH8 CH1 CH0
    g_fpga_sys_xadc_reg_mem->seq4_ch_bu         = 0;                          // all channels are unipolar
    g_fpga_sys_xadc_reg_mem->seq5_ch_bu         = 0;                          // all channels are unipolar
    g_fpga_sys_xadc_reg_mem->seq6_ch_st         = 0;                          // all channels have no additional settling time
    g_fpga_sys_xadc_reg_mem->seq7_ch_st         = 0;                          // all channels have no additional settling time

    /* all following alarm temperatures are taken from the BD system settings */
    g_fpga_sys_xadc_reg_mem->alarm00_tmp_hi     = 0xB5ED;                     // Temp alarm trigger
    g_fpga_sys_xadc_reg_mem->alarm01_vccint_hi  = 0x57E4;                     // Vccint upper alarm limit
    g_fpga_sys_xadc_reg_mem->alarm02_vccaux_hi  = 0xA147;                     // Vccaux upper alarm limit
    g_fpga_sys_xadc_reg_mem->alarm03_ot_lim     = 0xCA33;                     // Temp alarm OT upper
    g_fpga_sys_xadc_reg_mem->alarm04_tmp_lo     = 0xA93A;                     // Temp alarm reset
    g_fpga_sys_xadc_reg_mem->alarm05_vccint_lo  = 0x52C6;                     // Vccint lower alarm limit
    g_fpga_sys_xadc_reg_mem->alarm06_vccaux_lo  = 0x9555;                     // Vccaux lower alarm limit
    g_fpga_sys_xadc_reg_mem->alarm07_ot_rst     = 0xAE4E;                     // Temp alarm OT reset
    g_fpga_sys_xadc_reg_mem->alarm08_vccbram_hi = 0x5999;                     // Vccbram upper alarm limit
    g_fpga_sys_xadc_reg_mem->alarm09_vccpint_hi = 0x5555;                     // Vccpint upper alarm limit
    g_fpga_sys_xadc_reg_mem->alarm10_vccpaux_hi = 0x9999;                     // Vccpaux upper alarm limit
    g_fpga_sys_xadc_reg_mem->alarm11_vccoddr_hi = 0x6AAA;                     // Vccddro upper alarm limit
    g_fpga_sys_xadc_reg_mem->alarm12_vccbram_lo = 0x5111;                     // Vccbram lower alarm limit
    g_fpga_sys_xadc_reg_mem->alarm13_vccpint_lo = 0x5111;                     // Vccpint lower alarm limit
    g_fpga_sys_xadc_reg_mem->alarm14_vccpaux_lo = 0x91EB;                     // Vccpaux lower alarm limit
    g_fpga_sys_xadc_reg_mem->alarm15_vccoddr_lo = 0x6666;                     // Vccddro lower alarm limit

    {
        uint32_t temp = g_fpga_sys_xadc_reg_mem->stat_temp;
        fprintf(stderr, "DEBUG fpga_sys_xadc_init: 3) temp value of XADC register = %4.2f°C.\n", (((float) temp) * 503.975f / 65536.0f) - 273.15);
    }

    //fprintf(stderr, "fpga_sys_xadc_init: END\n");
    return 0;
}

/*----------------------------------------------------------------------------*/
int fpga_sys_xadc_exit(void)
{
    //fprintf(stderr, "fpga_sys_xadc_exit: BEGIN\n");

    if (g_fpga_sys_xadc_reg_mem) {
        /* reset the AXI XADC IP core */
        g_fpga_sys_xadc_reg_mem->srr                = 0x0000000A;                 // taking 16 clocks
        usleep(1000);
        g_fpga_sys_xadc_reg_mem->gier               = 0x00000000;                 // global interrupt disabled
    }

#if 0
    /* unmaps from the system GPIO LEDs module */
    if (fpga_munmap_area(&g_fpga_sys_gpio_leds_mem_fd, (void**) &g_fpga_sys_gpio_leds_reg_mem, FPGA_SYS_GPIO_LEDS_BASE_ADDR, FPGA_SYS_GPIO_LEDS_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_sys_xadc_exit: g_fpga_sys_gpio_leds_reg_mem - munmap() failed: %s\n", strerror(errno));
    }

    /* unmaps from the system GPIO XADC module */
    if (fpga_munmap_area(&g_fpga_sys_gpio_xadc_mem_fd, (void**) &g_fpga_sys_gpio_xadc_reg_mem, FPGA_SYS_GPIO_XADC_BASE_ADDR, FPGA_SYS_GPIO_XADC_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_sys_xadc_exit: g_fpga_sys_gpio_xadc_reg_mem - munmap() failed: %s\n", strerror(errno));
    }
#endif

    /* unmaps from the system XADC module */
    if (fpga_munmap_area(&g_fpga_sys_xadc_mem_fd, (void**) &g_fpga_sys_xadc_reg_mem, FPGA_SYS_XADC_BASE_ADDR, FPGA_SYS_XADC_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_sys_xadc_exit: g_fpga_sys_xadc_reg_mem - munmap() failed: %s\n", strerror(errno));
    }

    //fprintf(stderr, "fpga_sys_xadc_exit: END\n");
    return 0;
}
