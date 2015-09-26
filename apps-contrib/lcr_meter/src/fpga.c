/** @file fpga.c
 *
 * $Id: fpga.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope FPGA Interface.
 * @author Jure Menart <juremenart@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
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

#include "fpga.h"


/* @brief Pointer to FPGA control registers. */
static osc_fpga_reg_mem_t *g_osc_fpga_reg_mem = NULL;

/* @brief Pointer to data buffer where signal on channel A is captured.  */
static uint32_t           *g_osc_fpga_cha_mem = NULL;

/* @brief Pointer to data buffer where signal on channel B is captured.  */
static uint32_t           *g_osc_fpga_chb_mem = NULL;

/* @brief The memory file descriptor used to mmap() the FPGA space. */
static int                 g_osc_fpga_mem_fd = -1;

/* @brief Number of ADC acquisition bits.  */
const int                  c_osc_fpga_adc_bits = 14;

/* @brief Sampling frequency = 125Mspmpls (non-decimated). */
const float                c_osc_fpga_smpl_freq = 125e6;

/* @brief Sampling period (non-decimated) - 8 [ns]. */
const float                c_osc_fpga_smpl_period = (1. / 125e6);


/*----------------------------------------------------------------------------*/
/**
 * @brief Cleanup access to FPGA memory buffers
 *
 * Function optionally cleanups access to FPGA memory buffers, i.e. if access
 * has already been established it unmaps logical memory regions and close apparent
 * file descriptor.
 *
 * @retval  0 Success
 * @retval -1 Failure, error message is printed on standard error device
 */
static int __osc_fpga_cleanup_mem(void)
{
    /* optionally unmap memory regions  */
    if (g_osc_fpga_reg_mem) {
        if (munmap(g_osc_fpga_reg_mem, OSC_FPGA_BASE_SIZE) < 0) {
            fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
            return -1;
        }
        /* ...and update memory pointers */
        g_osc_fpga_reg_mem = NULL;
        g_osc_fpga_cha_mem = NULL;
        g_osc_fpga_chb_mem = NULL;
    }

    /* optionally close file descriptor */
    if(g_osc_fpga_mem_fd >= 0) {
        close(g_osc_fpga_mem_fd);
        g_osc_fpga_mem_fd = -1;
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Initialize interface to Oscilloscope FPGA module
 *
 * Function first optionally cleanups previously established access to Oscilloscope
 * FPGA module. Afterwards a new connection to the Memory handler is instantiated
 * by opening file descriptor over /dev/mem device. Access to Oscilloscope FPGA module
 * is further provided by mapping memory regions through resulting file descriptor.
 *
 * @retval  0 Success
 * @retval -1 Failure, error message is printed on standard error device
 *
 */
int osc_fpga_init(void)
{
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    /* If maybe needed, cleanup the FD & memory pointer */
    if(__osc_fpga_cleanup_mem() < 0)
        return -1;

    g_osc_fpga_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(g_osc_fpga_mem_fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    page_addr = OSC_FPGA_BASE_ADDR & (~(page_size-1));
    page_off  = OSC_FPGA_BASE_ADDR - page_addr;

    page_ptr = mmap(NULL, OSC_FPGA_BASE_SIZE, PROT_READ | PROT_WRITE,
                          MAP_SHARED, g_osc_fpga_mem_fd, page_addr);
    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
        __osc_fpga_cleanup_mem();
        return -1;
    }
    g_osc_fpga_reg_mem = page_ptr + page_off;
    g_osc_fpga_cha_mem = (uint32_t *)g_osc_fpga_reg_mem + 
        (OSC_FPGA_CHA_OFFSET / sizeof(uint32_t));
    g_osc_fpga_chb_mem = (uint32_t *)g_osc_fpga_reg_mem + 
        (OSC_FPGA_CHB_OFFSET / sizeof(uint32_t));

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Finalize and release allocated resources while accessing the
 * Oscilloscope FPGA module
 *
 * Function is intended to be  called at the program termination.
 *
 * @retval 0 Success, never fails
 */
int osc_fpga_exit(void)
{
    __osc_fpga_cleanup_mem();

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Setup Oscilloscope FPGA module based on specified settings
 * FPGA module
 *
 * @param[in] trig_imm           nonzero if acquisition is applied immediately, zero if acquisition is trigger dependent
 * @param[in] trig_source        0 ChannelA, 1 ChannelB, 2 External
 * @param[in] trig_edge          0 Positive, 1 Negative edge
 * @param[in] trig_delay         Number of signal data atoms to be captured after the trigger
 * @param[in] trig_level         Trigger threshold level. expressed in [V]
 * @param[in] time_range         Time range, expressed with value 0..5, defines capture decimation  1,8, 64,1024,8192,65536
 * @param[in] ch1_adc_max_v      Maximal voltage in [V] on ADC inputs for channel A
 * @param[in] ch2_adc_max_v      Maximal voltage in [V] on ADC inputs for channel B
 * @param[in] ch1_calib_dc_off   Calibrated DC offset on channel A, expressed in ADC counts
 * @param[in] ch1_user_dc_off    User defined DC offset on channel A, expressed in [V]
 * @param[in] ch2_calib_dc_off   Calibrated DC offset on channel B, expressed in ADC counts
 * @param[in] ch2_user_dc_off    User defined DC offset on channel B, expressed in [V]
 * @param[in] ch1_probe_att      Channel A Attenuation
 * @param[in] ch2_probe_att      Channel B Attenuation
 * @param[in] enable_avg_at_dec  Apply average calculation during decimation
 *
 * @retval  0 Success
 * @retval -1 Failure, error message is output on standard error device
 */

int osc_fpga_update_params(int trig_imm, int trig_source, int trig_edge, 
                           float trig_delay, float trig_level, int time_range,
                           float ch1_adc_max_v, float ch2_adc_max_v,
                           int ch1_calib_dc_off, float ch1_user_dc_off,
                           int ch2_calib_dc_off, float ch2_user_dc_off,
                           int ch1_probe_att, int ch2_probe_att,
			    int ch1_gain, int ch2_gain,
                           int enable_avg_at_dec)
{
    /* TODO: Locking of memory map */
    int fpga_trig_source = osc_fpga_cnv_trig_source(trig_imm, trig_source, 
                                                    trig_edge);
    int fpga_dec_factor = osc_fpga_cnv_time_range_to_dec(time_range);
    int fpga_delay;
    float after_trigger; /* how much after trigger FPGA should write */
    int fpga_trig_thr;
    
    uint32_t gain_hi_cha_filt_aa=0x7D93;
    uint32_t gain_hi_cha_filt_bb=0x437C7;
    uint32_t gain_hi_cha_filt_pp=0x2666;
    uint32_t gain_hi_cha_filt_kk=0xd9999a;
    
    uint32_t gain_hi_chb_filt_aa=0x7D93;
    uint32_t gain_hi_chb_filt_bb=0x437C7;
    uint32_t gain_hi_chb_filt_pp=0x2666;
    uint32_t gain_hi_chb_filt_kk=0xd9999a;
    
    uint32_t gain_lo_cha_filt_aa=0x4C5F;
    uint32_t gain_lo_cha_filt_bb=0x2F38B;
    uint32_t gain_lo_cha_filt_pp=0x2666;
    uint32_t gain_lo_cha_filt_kk=0xd9999a;
    
    uint32_t gain_lo_chb_filt_aa=0x4C5F;
    uint32_t gain_lo_chb_filt_bb=0x2F38B;
    uint32_t gain_lo_chb_filt_pp=0x2666;
    uint32_t gain_lo_chb_filt_kk=0xd9999a;    
    
  
    

    

    if(trig_source == 0) {
        
            

        fpga_trig_thr = osc_fpga_cnv_v_to_cnt(trig_level, ch1_adc_max_v,
                                              ch1_calib_dc_off, ch1_user_dc_off);
        //fprintf(stderr, "Trigger source [V] -> cnts: %f -> %d (max ADC V: %f)\n", trig_level, fpga_trig_thr, ch1_adc_max_v);
    } else {
       
        

        fpga_trig_thr = osc_fpga_cnv_v_to_cnt(trig_level, ch2_adc_max_v,
                                              ch2_calib_dc_off, ch2_user_dc_off);
        //fprintf(stderr, "Trigger source [V] -> cnts: %f -> %d (max ADC V: %f)\n", trig_level, fpga_trig_thr, ch2_adc_max_v);
    }

    if((fpga_trig_source < 0) || (fpga_dec_factor < 0)) {
        fprintf(stderr, "osc_fpga_update_params() failed\n");
        return -1;
    }

    /* Pre-trigger - we need to limit after trigger acquisition so we can
     * readout historic (pre-trigger) values */
    /* TODO: Bug in FPGA? We need to put at least 3 less samples to trig_delay */
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

    g_osc_fpga_reg_mem->other = enable_avg_at_dec;
    
    
    // Updating hysteresys registers
    
    
    g_osc_fpga_reg_mem->cha_hystersis=OSC_HYSTERESIS;
    g_osc_fpga_reg_mem->chb_hystersis=OSC_HYSTERESIS;
    
    
    // Updating equalization filter with default coefficients
    if (ch1_gain==0)
    {
     g_osc_fpga_reg_mem->cha_filt_aa =gain_hi_cha_filt_aa;
     g_osc_fpga_reg_mem->cha_filt_bb =gain_hi_cha_filt_bb;
     g_osc_fpga_reg_mem->cha_filt_pp =gain_hi_cha_filt_pp;
     g_osc_fpga_reg_mem->cha_filt_kk =gain_hi_cha_filt_kk;
    }
    else
    {
     g_osc_fpga_reg_mem->cha_filt_aa =gain_lo_cha_filt_aa;
     g_osc_fpga_reg_mem->cha_filt_bb =gain_lo_cha_filt_bb;
     g_osc_fpga_reg_mem->cha_filt_pp =gain_lo_cha_filt_pp;
     g_osc_fpga_reg_mem->cha_filt_kk =gain_lo_cha_filt_kk;      
    }
    
       if (ch2_gain==0)
    {
     g_osc_fpga_reg_mem->chb_filt_aa =gain_hi_chb_filt_aa;
     g_osc_fpga_reg_mem->chb_filt_bb =gain_hi_chb_filt_bb;
     g_osc_fpga_reg_mem->chb_filt_pp =gain_hi_chb_filt_pp;
     g_osc_fpga_reg_mem->chb_filt_kk =gain_hi_chb_filt_kk;
    }
    else
    {
     g_osc_fpga_reg_mem->chb_filt_aa =gain_lo_chb_filt_aa;
     g_osc_fpga_reg_mem->chb_filt_bb =gain_lo_chb_filt_bb;
     g_osc_fpga_reg_mem->chb_filt_pp =gain_lo_chb_filt_pp;
     g_osc_fpga_reg_mem->chb_filt_kk =gain_lo_chb_filt_kk;      
    }
    
    

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Reset write state machine
 *
 * @retval 0 Success, never fails
 */
int osc_fpga_reset(void)
{
    g_osc_fpga_reg_mem->conf |= OSC_FPGA_CONF_RST_BIT;
    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Arm the system to capture signal on next trigger
 *
 * @retval 0 Success, never fails
 */
int osc_fpga_arm_trigger(void)
{
    g_osc_fpga_reg_mem->conf |= OSC_FPGA_CONF_ARM_BIT;

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Define the trigger source
 *
 * @param[in]  trig_source Trigger source
 * @retval 0 Success, never fails
 */
int osc_fpga_set_trigger(uint32_t trig_source)
{
    g_osc_fpga_reg_mem->trig_source = trig_source;
    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Setup the trigger delay
 *
 * @param[in]  trig_delay Trigger delay, expressed in ADC samples
 * @retval 0 Success, never fails
 */
int osc_fpga_set_trigger_delay(uint32_t trig_delay)
{
    g_osc_fpga_reg_mem->trigger_delay = trig_delay;
    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Determine the "Trigger mode" the system is running in
 *
 * @retval 0 The system is not running in the trigger mode
 * @retval 1 The system is running in the trigger mode
 */
int osc_fpga_triggered(void)
{
    return ((g_osc_fpga_reg_mem->trig_source & OSC_FPGA_TRIG_SRC_MASK)==0);
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Retrieve the address of channel A,B memory buffers
 *
 * NOTE: no check is made if argumments are correctly specified.
 *
 * @param[out] cha_signal Pointer to channel A memory buffer
 * @param[out] chb_signal Pointer to channel B memory buffer
 * @retval 0 Success, never fails
 */
int osc_fpga_get_sig_ptr(int **cha_signal, int **chb_signal)
{
    *cha_signal = (int *)g_osc_fpga_cha_mem;
    *chb_signal = (int *)g_osc_fpga_chb_mem;
    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Retrieve Memory buffer Write pointer information
 *
 * @param[out] wr_ptr_curr offset to the currently captured signal atom
 * @param[out] wr_ptr_trig offset to signal atom, captured at detected trigger
 * @retval 0 Success, never fails
 */
int osc_fpga_get_wr_ptr(int *wr_ptr_curr, int *wr_ptr_trig)
{
    if(wr_ptr_curr)
        *wr_ptr_curr = g_osc_fpga_reg_mem->wr_ptr_cur;
    if(wr_ptr_trig)
        *wr_ptr_trig = g_osc_fpga_reg_mem->wr_ptr_trigger;
    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Convert specified trigger settings into FPGA control value
 *
 * The specified trigger settings are converted into a value, which need to
 * be applied into trigger_source FPGA register in order the specified settings
 * becomes active
 *
 * @param[in] trig_imm     nonzero if acquisition is applied immediately, zero if acquisition is trigger dependent
 * @param[in] trig_source  0 ChannelA, 1 ChannelB, 2 External
 * @param[in] trig_edge    0 Positive, 1 Negative edge
 *
 * @retval -1 failure, indicating invalid arguments are specified
 * @retval >0, <8 Control value to be written into trigger_source FPGA register
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


/*----------------------------------------------------------------------------*/
/**
 * @brief Converts time_range parameter to decimation factor
 *
 *
 * @param[in] time_range        time range parameter [0..5]
 * @retval    -1                failure, indicating invalid arguments are specified
 * @retval     1,8,64,1K,8K,64K decimation factor
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

/*----------------------------------------------------------------------------*/
/**
 * @brief Converts time in [s] to ADC samples
 *
 *
 * @param[in] time        time, specified in [s]
 * @param[in] dec_factor  decimation factor
 * @retval    int         number of ADC samples
 */
int osc_fpga_cnv_time_to_smpls(float time, int dec_factor)
{
    /* Calculate sampling period (including decimation) */
    float smpl_p = (c_osc_fpga_smpl_period * dec_factor);
    int fpga_smpls = (int)round(time / smpl_p);

    return fpga_smpls;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Converts voltage in [V] to ADC counts
 *
 * Function is used for setting up trigger threshold value, which is written into
 * appropriate FPGA register. This value needs to be specified in ADC counts, while
 * user specifies this information in Voltage. The resulting value is based on the
 * specified threshold voltage, maximal ADC voltage, calibrated and user specified
 * DC offsets.
 *
 * @param[in] voltage        Voltage, specified in [V]
 * @param[in] adc_max_v      Maximal ADC voltage, specified in [V]
 * @param[in] calib_dc_off   Calibrated DC offset, specified in ADC counts
 * @param[in] user_dc_off    User specified DC offset, , specified in [V]
 * @retval    int            ADC counts
 */
int osc_fpga_cnv_v_to_cnt(float voltage, float adc_max_v,
                          int calib_dc_off, float user_dc_off)
{
    int adc_cnts = 0;

    /* check and limit the specified voltage arguments towards */
    /* maximal voltages which can be applied on ADC inputs     */
    if(voltage > adc_max_v)
        voltage = adc_max_v;
    else if(voltage < -adc_max_v)
        voltage = -adc_max_v;

    /* adopt the specified voltage with user defined DC offset */
    voltage -= user_dc_off;

    /* map voltage units into FPGA adc counts */
    adc_cnts = (int)round(voltage * (float)((int)(1<<c_osc_fpga_adc_bits)) / 
                          (2*adc_max_v));

    /* clip to the highest value (we are dealing with 14 bits only) */
    if((voltage > 0) && (adc_cnts & (1<<(c_osc_fpga_adc_bits-1))))
        adc_cnts = (1<<(c_osc_fpga_adc_bits-1))-1;
    else
        adc_cnts = adc_cnts & ((1<<(c_osc_fpga_adc_bits))-1);

    /* adopt calculated ADC counts with calibration DC offset */
    adc_cnts -= calib_dc_off;

    return adc_cnts;
}



/*----------------------------------------------------------------------------*/
/**
 * @brief Converts ADC counts to voltage [V]
 *
 * Function is used to publish captured signal data to external world in user units.
 * Calculation is based on maximal voltage, which can be applied on ADC inputs and
 * calibrated and user defined DC offsets.
 *
 * @param[in] cnts           Captured Signal Value, expressed in ADC counts
 * @param[in] adc_max_v      Maximal ADC voltage, specified in [V]
 * @param[in] calib_dc_off   Calibrated DC offset, specified in ADC counts
 * @param[in] user_dc_off    User specified DC offset, specified in [V]
 * @retval    float          Signal Value, expressed in user units [V]
 */
float osc_fpga_cnv_cnt_to_v(int cnts, float adc_max_v,
                            int calib_dc_off, float user_dc_off)
{
    int m;
    float ret_val;

    /* check sign */
    if(cnts & (1<<(c_osc_fpga_adc_bits-1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1<<c_osc_fpga_adc_bits)-1)) + 1);
    } else {
        /* positive number */
        m = cnts;
    }

    /* adopt ADC count with calibrated DC offset */
    m += calib_dc_off;

    /* map ADC counts into user units */
    if(m < (-1 * (1<<(c_osc_fpga_adc_bits-1))))
        m = (-1 * (1<<(c_osc_fpga_adc_bits-1)));
    else if(m > (1<<(c_osc_fpga_adc_bits-1)))
        m =  (1<<(c_osc_fpga_adc_bits-1));

    ret_val =  (m * adc_max_v / 
                (float)(1<<(c_osc_fpga_adc_bits-1)));

    /* and adopt the calculation with user specified DC offset */
    ret_val += user_dc_off;

    return ret_val;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Calculates maximum [V] in respect to set & calibration parameters
 *
 * Function is used to calculate the maximal voltage which can be applied on ADC inputs.
 * This calculation is based on calibrated Front End Full Scale Gain setting and configured
 * Probe Attenuation.
 *
 * @param[in] fe_gain_fs     Front End Full Scale Gain
 * @param[in] probe_att      Probe attenuation
 * @retval    float          Maximum voltage, expressed in [V]
 */
/*  */
float osc_fpga_calc_adc_max_v(uint32_t fe_gain_fs, int probe_att)
{
    float max_adc_v;
    int probe_att_fact = (probe_att > 0) ? 10 : 1;

    max_adc_v = 
        fe_gain_fs/(float)((uint64_t)1<<32) * 100 * (probe_att_fact);

    return max_adc_v;
}
