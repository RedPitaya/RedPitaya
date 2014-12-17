/** 
 * $Id: fpga_osc.c 881 2013-12-16 05:37:34Z rp_jmenart $
 * 
 * @brief Red Pitaya Oscilloscope (OSC) FPGA controller.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 *
 * For all functions from standard C library it is easy to see the description
 * using the manuals with 'man' command in GNU/Linux console. For example, to see
 * the description, usage and examples of function 'mmap()' use command:
 * 'man mmap'
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

#include "fpga_osc.h"

/**
 * GENERAL DESCRIPTION:
 * 
 * This module initializes and provides for other SW modules the access to the
 * FPGA OSC module. The oscilloscope memory space is divided to three parts:
 *   - registers (control and status)
 *   - input signal buffer (Channel A)
 *   - input signal buffer (Channel B)
 * 
 * This module maps physical address of the oscilloscope core to the logical
 * address, which can be used in the GNU/Linux user-space. To achieve this,
 * OSC_FPGA_BASE_ADDR from CPU memory space is translated automatically to 
 * logical address with the function mmap(). After all the initialization is done,
 * other modules can use this module to controll oscilloscope FPGA core. Before
 * any functions or functionality from this module can be used it needs to be
 * initialized with osc_fpga_init() function. When this module is no longer
 * needed osc_fpga_exit() must be called.
 *  
 * FPGA oscilloscope state machine in various modes. Basic principle is that 
 * SW sets the machine, 'arm' the writting machine (FPGA writes from ADC to
 * input buffer memory) and then set the triggers. FPGA machine is continue 
 * writting to the buffers until the trigger is detected plus the amount set
 * in trigger delay register. For more detauled description see the FPGA OSC
 * registers description.
 * 
 * Nice example how to use this module can be seen in worker.c module.
 */

/* internal structures */
/** The FPGA register structure (defined in fpga_osc.h) */
osc_fpga_reg_mem_t *g_osc_fpga_reg_mem = NULL;
/** The FPGA input signal buffer pointer for channel A */
uint32_t           *g_osc_fpga_cha_mem = NULL;
/** The FPGA input signal buffer pointer for channel B */
uint32_t           *g_osc_fpga_chb_mem = NULL;

/** The memory file descriptor used to mmap() the FPGA space */
int             g_osc_fpga_mem_fd = -1;

/* Constants */
/** ADC number of bits */
const int c_osc_fpga_adc_bits = 14;
/** @brief Max and min voltage on ADCs. 
 * Symetrical - Max Voltage = +14, Min voltage = -1 * c_osc_fpga_max_v 
 */
const float c_osc_fpga_adc_max_v  = +14;
/** Sampling frequency = 125Mspmpls (non-decimated) */
const float c_osc_fpga_smpl_freq = 125e6;
/** Sampling period (non-decimated) - 8 [ns] */
const float c_osc_fpga_smpl_period = (1. / 125e6);

/**
 * @brief Internal function used to clean up memory.
 * 
 * This function un-maps FPGA register and signal buffers, closes memory file
 * descriptor and cleans all memory allocated by this module.
 *
 * @retval 0 Success
 * @retval -1 Error happened during cleanup.
 */
int __osc_fpga_cleanup_mem(void)
{
    /* If register structure is NULL we do not need to un-map and clean up */
    if(g_osc_fpga_reg_mem) {
        if(munmap(g_osc_fpga_reg_mem, OSC_FPGA_BASE_SIZE) < 0) {
            fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
            return -1;
        }
        g_osc_fpga_reg_mem = NULL;
        if(g_osc_fpga_cha_mem)
            g_osc_fpga_cha_mem = NULL;
        if(g_osc_fpga_chb_mem)
            g_osc_fpga_chb_mem = NULL;
    }
    if(g_osc_fpga_mem_fd >= 0) {
        close(g_osc_fpga_mem_fd);
        g_osc_fpga_mem_fd = -1;
    }
    return 0;
}

/**
 * @brief Maps FPGA memory space and prepares register and buffer variables.
 * 
 * This function opens memory device (/dev/mem) and maps physical memory address
 * OSC_FPGA_BASE_ADDR (of length OSC_FPGA_BASE_SIZE) to logical addresses. It 
 * initializes the pointers g_osc_fpga_reg_mem, g_osc_fpga_cha_mem and
 * g_osc_fpga_chb_mem to point to FPGA OSC.
 * If function failes FPGA variables must not be used.
 *
 * @retval 0  Success
 * @retval -1 Failure, error is printed to standard error output.
 */
int osc_fpga_init(void)
{
    /* Page variables used to calculate correct mapping addresses */
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    /* If module was already initialized once, clean all internals. */
    if(__osc_fpga_cleanup_mem() < 0)
        return -1;

    /* Open /dev/mem to access directly system memory */
    g_osc_fpga_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(g_osc_fpga_mem_fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    /* Calculate correct page address and offset from OSC_FPGA_BASE_ADDR and
     * OSC_FPGA_BASE_SIZE 
     */
    page_addr = OSC_FPGA_BASE_ADDR & (~(page_size-1));
    page_off  = OSC_FPGA_BASE_ADDR - page_addr;

    /* Map FPGA memory space to page_ptr. */
    page_ptr = mmap(NULL, OSC_FPGA_BASE_SIZE, PROT_READ | PROT_WRITE,
                          MAP_SHARED, g_osc_fpga_mem_fd, page_addr);
    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
        __osc_fpga_cleanup_mem();
        return -1;
    }

    /* Set FPGA OSC module pointers to correct values. */
    g_osc_fpga_reg_mem = page_ptr + page_off;
    g_osc_fpga_cha_mem = (uint32_t *)g_osc_fpga_reg_mem + 
        (OSC_FPGA_CHA_OFFSET / sizeof(uint32_t));
    g_osc_fpga_chb_mem = (uint32_t *)g_osc_fpga_reg_mem + 
        (OSC_FPGA_CHB_OFFSET / sizeof(uint32_t));

    return 0;
}

/**
 * @brief Cleans up FPGA OSC module internals.
 * 
 * This function closes the memory file descriptor, unmap the FPGA memory space
 * and cleans also all other internal things from FPGA OSC module.
 * @retval 0 Sucess
 * @retval -1 Failure
 */
int osc_fpga_exit(void)
{
    return __osc_fpga_cleanup_mem();
}

/**
 * @brief Updates triggering parameters in FPGA registers.
 *
 * This function updates trigger related parameters in FPGA registers.
 *
 * @param [in] trig_imm Trigger immediately - if set to 1, FPGA state machine 
 *                      will trigger immediately and other trigger parameters
 *                      will be ignored.
 * @param [in] trig_source Trigger source, as defined in rp_main_params.
 * @param [in] trig_edge Trigger edge, as defined in rp_main_params.
 * @param [in] trig_delay Trigger delay in [s].
 * @param [in] trig_level Trigger level in [V].
 * @param [in] time_range Time range, as defined in rp_main_params.
 *
 * @retval 0 Success
 * @retval -1 Failure
 * 
 * @see rp_main_params
 */
int osc_fpga_update_params(int trig_imm, int trig_source, int trig_edge, 
                           float trig_delay, float trig_level, int time_range)
{
    int fpga_trig_source = osc_fpga_cnv_trig_source(trig_imm, trig_source, 
                                                    trig_edge);
    int fpga_dec_factor = osc_fpga_cnv_time_range_to_dec(time_range);
    int fpga_delay;
    float after_trigger; /* how much after trigger FPGA should write */
    int fpga_trig_thr = osc_fpga_cnv_v_to_cnt(trig_level);
    
    
    uint32_t gain_hi_cha_filt_aa=0x7D93;
    uint32_t gain_hi_cha_filt_bb=0x437C7;
    uint32_t gain_hi_cha_filt_pp=0x2666;
    uint32_t gain_hi_cha_filt_kk=0xd9999a;
    
    uint32_t gain_hi_chb_filt_aa=0x7D93;
    uint32_t gain_hi_chb_filt_bb=0x437C7;
    uint32_t gain_hi_chb_filt_pp=0x2666;
    uint32_t gain_hi_chb_filt_kk=0xd9999a;
    
    
    
    
    
    

    if((fpga_trig_source < 0) || (fpga_dec_factor < 0)) {
        fprintf(stderr, "osc_fpga_update_params() failed\n");
        return -1;
    }

    /* Pre-trigger - we need to limit after trigger acquisition so we can
     * readout historic (pre-trigger) values */
    
    if (trig_imm)
      after_trigger=OSC_FPGA_SIG_LEN* c_osc_fpga_smpl_period * fpga_dec_factor;
    else
    after_trigger = 
        ((OSC_FPGA_SIG_LEN-7) * c_osc_fpga_smpl_period * fpga_dec_factor) +
        trig_delay;

    if(after_trigger < 0)
        after_trigger = 0;

    fpga_delay = osc_fpga_cnv_time_to_smpls(after_trigger, fpga_dec_factor);

    /* Trig source is written after ARM */
    /*    g_osc_fpga_reg_mem->trig_source   = fpga_trig_source;*/
    if(trig_source == 0) 
        g_osc_fpga_reg_mem->cha_thr   = fpga_trig_thr;
    else
        g_osc_fpga_reg_mem->chb_thr   = fpga_trig_thr;
    g_osc_fpga_reg_mem->data_dec      = fpga_dec_factor;
    g_osc_fpga_reg_mem->trigger_delay = (uint32_t)fpga_delay;
    
    
    // Updating equalization filter with default coefficients  
    g_osc_fpga_reg_mem->cha_filt_aa =gain_hi_cha_filt_aa;
    g_osc_fpga_reg_mem->cha_filt_bb =gain_hi_cha_filt_bb;
    g_osc_fpga_reg_mem->cha_filt_pp =gain_hi_cha_filt_pp;
    g_osc_fpga_reg_mem->cha_filt_kk =gain_hi_cha_filt_kk;
    
    g_osc_fpga_reg_mem->chb_filt_aa =gain_hi_chb_filt_aa;
    g_osc_fpga_reg_mem->chb_filt_bb =gain_hi_chb_filt_bb;
    g_osc_fpga_reg_mem->chb_filt_pp =gain_hi_chb_filt_pp;
    g_osc_fpga_reg_mem->chb_filt_kk =gain_hi_chb_filt_kk;     
    
     
    return 0;
}

/** @brief OSC FPGA reset
 * 
 * Triggers internal oscilloscope FPGA state machine reset.
 *
 * @retval 0 Always returns 0.
 */
int osc_fpga_reset(void)
{
    g_osc_fpga_reg_mem->conf |= OSC_FPGA_CONF_RST_BIT;
    return 0;
}

/** @brief OSC FPGA ARM
 *
 * ARM internal oscilloscope FPGA state machine to start writting input buffers.

 * @retval 0 Always returns 0.
 */
int osc_fpga_arm_trigger(void)
{
    g_osc_fpga_reg_mem->conf |= OSC_FPGA_CONF_ARM_BIT;

    return 0;
}

/** @brief Sets the trigger source in OSC FPGA register.
 *
 * Sets the trigger source in oscilloscope FPGA register. 
 *
 * @param [in] trig_source Trigger source, as defined in FPGA register 
 *                         description.
 */
int osc_fpga_set_trigger(uint32_t trig_source)
{
    g_osc_fpga_reg_mem->trig_source = trig_source;
    return 0;
}

/** @brief Sets the trigger delay in OSC FPGA register.
 *
 * Sets the trigger delay in oscilloscope FPGA register. 
 *
 * @param [in] trig_delay Trigger delay, as defined in FPGA register 
 *                         description.
 * 
 * @retval 0 Always returns 0.
 */
int osc_fpga_set_trigger_delay(uint32_t trig_delay)
{
    g_osc_fpga_reg_mem->trigger_delay = trig_delay;
    return 0;
}

/** @brief Checks if FPGA detected trigger.
 *
 * This function checks if trigger was detected by the FPGA.
 *
 * @retval 0 Trigger not detected.
 * @retval 1 Trigger detected.
 */
int osc_fpga_triggered(void)
{
    return ((g_osc_fpga_reg_mem->trig_source & OSC_FPGA_TRIG_SRC_MASK)==0);
}

/** @brief Returns memory pointers for both input signal buffers.
 *
 * This function returns pointers for input signal buffers for both channels.
 * 
 * @param [out] cha_signal Output pointer for Channel A buffer
 * @param [out] cha_signal Output pointer for Channel B buffer
 * 
 * @retval 0 Always returns 0.
 */
int osc_fpga_get_sig_ptr(int **cha_signal, int **chb_signal)
{
    *cha_signal = (int *)g_osc_fpga_cha_mem;
    *chb_signal = (int *)g_osc_fpga_chb_mem;
    return 0;
}

/** @brief Returns values for current and trigger write FPGA pointers.
 *
 * This functions returns values for current and trigger write pointers. They
 * are an address of the input signal buffer and are the same for both channels.
 *
 * @param [out] wr_ptr_curr Current FPGA input buffer address.
 * @param [out] wr_ptr_trig Trigger FPGA input buffer address.
 *
 * @retval 0 Always returns 0.
  */
int osc_fpga_get_wr_ptr(int *wr_ptr_curr, int *wr_ptr_trig)
{
    if(wr_ptr_curr)
        *wr_ptr_curr = g_osc_fpga_reg_mem->wr_ptr_cur;
    if(wr_ptr_trig)
        *wr_ptr_trig = g_osc_fpga_reg_mem->wr_ptr_trigger;
    return 0;
}

/** @brief Convert trigger parameters to FPGA trigger source value.
 *
 * This function takes as an argument trigger parameters and converts it to
 * trigger source value used by the FPGA trigger source reigster.
 *
 * @param [in] trig_imm Trigger immediately, if set to 1 other trigger parameters
 *                      are ignored.
 * @param [in] trig_source Trigger source as defined in rp_main_params
 * @param [in] trig_edge Trigger edge as defined in rp_main_params
 * 
 * @retval -1 Error
 * @retval otherwise Trigger source FPGA value
  */
int osc_fpga_cnv_trig_source(int trig_imm, int trig_source, int trig_edge)
{
    int fpga_trig_source = 0;

    /* Trigger immediately */    
    if(trig_imm)
        return 1;

    switch(trig_source) {
    case 0: /* ChA*/
        if(trig_edge == 0)
            fpga_trig_source = 2;
        else
            fpga_trig_source = 3;
        break;
    case 1: /* ChB*/
        if(trig_edge == 0)
            fpga_trig_source = 4;
        else
            fpga_trig_source = 5;
        break;
    case 2: /* External */
        if(trig_edge == 0)
            fpga_trig_source = 6;
        else
            fpga_trig_source = 7;

        break;
    default:
        /* Error */
        return -1;
    }

    return fpga_trig_source;
}

/** @brief Converts time range to decimation value.
 *
 * This function converts time range value defined by rp_main_params to 
 * decimation factor value.
 *
 * @param [in] time_range Time range, integer between 0 and 5, as defined by
 *                        rp_main_params.
 * 
 * @retval -1 Error
 *
 * @retval otherwise Decimation factor.
*/
int osc_fpga_cnv_time_range_to_dec(int time_range)
{
    /* Input: 0, 1, 2, 3, 4, 5 translates to:
     * Output: 1x, 8x, 64x, 1kx, 8kx, 65kx */
    switch(time_range) {
    case 0:
        return 1;
        break;
    case 1:
        return 8;
        break;
    case 2:
        return 64;
        break;
    case 3:
        return 1024;
        break;
    case 4:
        return 8*1024;
        break;
    case 5:
        return 64*1024;
        break;
    default:
        return -1;
    }

    return -1;
}

/** @brief Converts time to number of samples.
 *
 * This function converts time in [s], based on current decimation factor to
 * number of samples at ADC sampling frequency.
 * 
 * @param [in] time Time in [s]
 * @param [in] dec_factor Decimation factor
 * 
 * @retval Number of ADC samples define dby input parameters.
 */
int osc_fpga_cnv_time_to_smpls(float time, int dec_factor)
{
    /* Calculate sampling period (including decimation) */
    float smpl_p = (c_osc_fpga_smpl_period * dec_factor);
    int fpga_smpls = (int)round(time / smpl_p);

    return fpga_smpls;
}

/** @brief Converts voltage to ADC counts.
 * 
 * This function converts voltage in [V] to ADC counts.
 * 
 * @param [in] voltage Voltage in [V]
 * 
 * @retval adc_cnts ADC counts
 */
int osc_fpga_cnv_v_to_cnt(float voltage)
{
    int adc_cnts = 0;

    if((voltage > c_osc_fpga_adc_max_v) || (voltage < -c_osc_fpga_adc_max_v))
        return -1;
    
    adc_cnts = (int)round(voltage * (float)((int)(1<<c_osc_fpga_adc_bits)) / 
                          (2*c_osc_fpga_adc_max_v));

    /* Clip highest value (+14 is calculated in int32_t to 0x2000, but we have
     * only 14 bits 
     */
    if((voltage > 0) && (adc_cnts & (1<<(c_osc_fpga_adc_bits-1))))
        adc_cnts = (1<<(c_osc_fpga_adc_bits-1))-1;
    else
        adc_cnts = adc_cnts & ((1<<(c_osc_fpga_adc_bits))-1);

    return adc_cnts;
}

/** @brief Converts ADC counts to voltage
 *
 * This function converts ADC counts to voltage (in [V])
 * 
 * @param [in] cnts ADC counts
 * 
 * @retval voltage Voltage in [V]
 */
float osc_fpga_cnv_cnt_to_v(int cnts)
{
    int m;

    if(cnts & (1<<(c_osc_fpga_adc_bits-1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1<<c_osc_fpga_adc_bits)-1)) + 1);
    } else {
        m = cnts;
        /* positive number */
    }
    return m;
}

