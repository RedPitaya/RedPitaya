/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer DSP LTI modified FPGA Interface 
 *
 * @Author Jure Menart <juremenart@gmail.com>
 * @Author Dashpi <dashpi46@gmail.com>
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

#include "fpga_lti.h"


/* internals */
/* The FPGA register structure */
lti_fpga_reg_mem_t *g_lti_fpga_reg_mem = NULL;

uint32_t           *g_lti_fpga_cha_mem = NULL;
uint32_t           *g_lti_fpga_chb_mem = NULL;



/* The memory file descriptor used to mmap() the FPGA space */
int             g_lti_fpga_mem_fd = -1;

/* constants */
/* ADC format = s.13 */
const int c_lti_fpga_adc_bits = 14;
/* c_osc_fpga_max_v */
float g_lti_fpga_adc_max_v;
const float c_lti_fpga_adc_max_v_revC= +1.079;
const float c_lti_fpga_adc_max_v_revD= +1.027;
/* Sampling frequency = 125Mspmpls (non-decimated) */
const float c_lti_fpga_smpl_freq = 125e6;
/* Sampling period (non-decimated) - 8 [ns] */
const float c_lti_fpga_smpl_period = (1. / 125e6);


double __rp_rand()
{
    return ((double)rand() / (double)RAND_MAX);
}

int __lti_fpga_cleanup_mem(void)
{
    if(g_lti_fpga_reg_mem) {
        if(munmap(g_lti_fpga_reg_mem, LTI_FPGA_BASE_SIZE) < 0) {
            fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
            return -1;
        }
        g_lti_fpga_reg_mem = NULL;
        if(g_lti_fpga_cha_mem)
            g_lti_fpga_cha_mem = NULL;
        if(g_lti_fpga_chb_mem)
            g_lti_fpga_chb_mem = NULL;

    }
    if(g_lti_fpga_mem_fd >= 0) {
        close(g_lti_fpga_mem_fd);
        g_lti_fpga_mem_fd = -1;
    }

    return 0;
}

static int get_hw_rev(hw_rev_t *hw_rev)
{
    void *page_ptr;
    long page_addr, page_size = sysconf(_SC_PAGESIZE);
    const long c_hk_fpga_base_addr = 0x40000000;
    const long c_hk_fpga_base_size = 0x20;
    int fd = -1;

    fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if(fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    page_addr = c_hk_fpga_base_addr & (~(page_size-1));

    page_ptr = mmap(NULL, c_hk_fpga_base_size, PROT_READ,
                          MAP_SHARED, fd, page_addr);

    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    hk_fpga_reg_mem_t *hk = page_ptr;
    *hw_rev = hk->rev & HK_FPGA_HW_REV_MASK;

    if(munmap(page_ptr, c_hk_fpga_base_size) < 0) {
        fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    close (fd);

    return 0;
}

static int update_hw_spec_par(void)
{
    hw_rev_t rev;
    if(get_hw_rev(&rev)<0){
    	return -1;
    }
    switch(rev){
    case eHwRevC:
    	g_lti_fpga_adc_max_v=c_lti_fpga_adc_max_v_revC;
    	break;
    case eHwRevD:
    	g_lti_fpga_adc_max_v=c_lti_fpga_adc_max_v_revD;
    	break;
    default:
    	return -1;
    break;
    }
    return 0;
}

int lti_fpga_init(void)
{
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    /* update hw specific parmateres */
    if(update_hw_spec_par()<0){
    	return -1;
    }

    /* If maybe needed, cleanup the FD & memory pointer */
    if(__lti_fpga_cleanup_mem() < 0)
        return -1;

    g_lti_fpga_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(g_lti_fpga_mem_fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    page_addr = LTI_FPGA_BASE_ADDR & (~(page_size-1));
    page_off  = LTI_FPGA_BASE_ADDR - page_addr;

    page_ptr = mmap(NULL, LTI_FPGA_BASE_SIZE, PROT_READ | PROT_WRITE,
                          MAP_SHARED, g_lti_fpga_mem_fd, page_addr);
    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
        __lti_fpga_cleanup_mem();
        return -1;
    }
    g_lti_fpga_reg_mem = page_ptr + page_off;
    g_lti_fpga_cha_mem = (uint32_t *)g_lti_fpga_reg_mem + 
        (LTI_FPGA_CHA_OFFSET / sizeof(uint32_t));
    g_lti_fpga_chb_mem = (uint32_t *)g_lti_fpga_reg_mem + 
        (LTI_FPGA_CHB_OFFSET / sizeof(uint32_t));
  

    return 0;
}

int lti_fpga_exit(void)
{
    __lti_fpga_cleanup_mem();

    return 0;
}

int lti_fpga_update_params(int trig_imm, int trig_source, int trig_edge, 
                           float trig_delay, float trig_level, int freq_range,
                           int enable_avg_at_dec)
{
    /* TODO: Locking of memory map */
    int fpga_trig_source = lti_fpga_cnv_trig_source(trig_imm, trig_source, 
                                                    trig_edge);
    int fpga_dec_factor = lti_fpga_cnv_freq_range_to_dec(freq_range);
    int fpga_delay;
    int fpga_trig_thr = lti_fpga_cnv_v_to_cnt(trig_level);

    /* Equalization filter coefficients */
    uint32_t gain_hi_cha_filt_aa = 0x7D93;
    uint32_t gain_hi_cha_filt_bb = 0x437C7;
    uint32_t gain_hi_cha_filt_pp = 0x0;
    uint32_t gain_hi_cha_filt_kk = 0xffffff;
    
    uint32_t gain_hi_chb_filt_aa = 0x7D93;
    uint32_t gain_hi_chb_filt_bb = 0x437C7;
    uint32_t gain_hi_chb_filt_pp = 0x0;
    uint32_t gain_hi_chb_filt_kk = 0xffffff;
    
    if((fpga_trig_source < 0) || (fpga_dec_factor < 0)) {
        fprintf(stderr, "lti_fpga_update_params() failed\n");
        return -1;
    }

    fpga_delay = LTI_FPGA_SIG_LEN - 3;

    /* Trig source is written after ARM */
    /*    g_lti_fpga_reg_mem->trig_source   = fpga_trig_source;*/
    if(trig_source == 0) 
        g_lti_fpga_reg_mem->cha_thr   = fpga_trig_thr;
    else
        g_lti_fpga_reg_mem->chb_thr   = fpga_trig_thr;

    g_lti_fpga_reg_mem->data_dec      = fpga_dec_factor;
    g_lti_fpga_reg_mem->trigger_delay = (uint32_t)fpga_delay;

    g_lti_fpga_reg_mem->other = enable_avg_at_dec;

    g_lti_fpga_reg_mem->cha_filt_aa = gain_hi_cha_filt_aa;
    g_lti_fpga_reg_mem->cha_filt_bb = gain_hi_cha_filt_bb;
    g_lti_fpga_reg_mem->cha_filt_pp = gain_hi_cha_filt_pp;
    g_lti_fpga_reg_mem->cha_filt_kk = gain_hi_cha_filt_kk;

    g_lti_fpga_reg_mem->chb_filt_aa = gain_hi_chb_filt_aa;
    g_lti_fpga_reg_mem->chb_filt_bb = gain_hi_chb_filt_bb;
    g_lti_fpga_reg_mem->chb_filt_pp = gain_hi_chb_filt_pp;
    g_lti_fpga_reg_mem->chb_filt_kk = gain_hi_chb_filt_kk;

    return 0;
}

int lti_fpga_reset(void)
{
    g_lti_fpga_reg_mem->conf |= LTI_FPGA_CONF_RST_BIT;
    return 0;
}

int lti_fpga_arm_trigger(void)
{
    g_lti_fpga_reg_mem->conf |= LTI_FPGA_CONF_ARM_BIT;

    return 0;
}

int lti_fpga_set_trigger(uint32_t trig_source)
{
    g_lti_fpga_reg_mem->trig_source = trig_source;
    return 0;
}

int lti_fpga_set_trigger_delay(uint32_t trig_delay)
{
    g_lti_fpga_reg_mem->trigger_delay = trig_delay;
    return 0;
}

int lti_fpga_triggered(void)
{
    return ((g_lti_fpga_reg_mem->trig_source & LTI_FPGA_TRIG_SRC_MASK)==0);
}

int lti_fpga_get_sig_ptr(int **cha_signal, int **chb_signal)
{
    *cha_signal = (int *)g_lti_fpga_cha_mem;
    *chb_signal = (int *)g_lti_fpga_chb_mem;
    return 0;
}

int lti_fpga_get_signal(double **cha_signal, double **chb_signal)
{
    int wr_ptr_trig;
    int in_idx, out_idx;
    double *cha_o = *cha_signal;
    double *chb_o = *chb_signal;

    if(!cha_o || !chb_o) {
        fprintf(stderr, "lti_fpga_get_signal() not initialized\n");
        return -1;
    }

    lti_fpga_get_wr_ptr(NULL, &wr_ptr_trig);

    for(in_idx = wr_ptr_trig + 1, out_idx = 0; 
        out_idx < LTI_FPGA_SIG_LEN; in_idx++, out_idx++) {
        if(in_idx >= LTI_FPGA_SIG_LEN)
            in_idx = in_idx % LTI_FPGA_SIG_LEN;

        cha_o[out_idx] = g_lti_fpga_cha_mem[in_idx];
        chb_o[out_idx] = g_lti_fpga_chb_mem[in_idx];

        // convert to signed
        if(cha_o[out_idx] > (double)(1<<13))
            cha_o[out_idx] -= (double)(1<<14);
        if(chb_o[out_idx] > (double)(1<<13))
            chb_o[out_idx] -= (double)(1<<14);
    }
    return 0;
}

int lti_fpga_online_dsp(double **ch1_data, double **ch2_data, int gen_delay,  
			   double **state_a, double **state_b, double **dsp_par_a, 
			   double **dsp_par_b, int dsp_loc_ptr, int **awg_a_ptr, 
			   int **awg_b_ptr)
{
    int curr_ptr, dsp_ptr;
    int in_idx, out_idx, in_loc, out_loc;
    double *cha_in = *ch1_data;  // Use this variables to pass DSP signals to GUI
    double *chb_in = *ch2_data;
    
    double *cha_state = *state_a;    //Array used to register DSP states between calls
    //double *chb_state = *state_b;  DSP enabled on channel 1 only

    double *cha_dsp_par = *dsp_par_a; // DSP parameters array
    
    double fir,iir,curr_in; // DSP variables    
    int order;   
    int dsp_ix;
     
    int *cha_awg=*awg_a_ptr;       
    //int *chb_awg=*awg_b_ptr;     DSP enabled on channel 1 only

    order= (int) (cha_dsp_par[64]); //IIR filter specific
    
    if(!cha_in || !chb_in) {
        fprintf(stderr, "lti_fpga_get_signal() not initialized\n");
        return -1;
    }  
     
     // Check current input write pointer (the buffer is running ("live" mode) ) 
      lti_fpga_get_wr_ptr(&curr_ptr, NULL);
    
    
   // Process the samples behind write pointer   
     
      
      // Resolve buffer wrapping  
      if ((curr_ptr)<dsp_loc_ptr)
      {
	curr_ptr=curr_ptr+LTI_FPGA_SIG_LEN;
      }
      //Now dsp_loc_ptr <= curr_ptr 
      
   
      
      //(loc. to be processed from dsp_loc_ptr to curr_ptr-1)  
      for(in_idx= dsp_loc_ptr, out_idx=0; in_idx<(curr_ptr-1); in_idx++, out_idx++) {
	
	// Resolve buffer wrapping
	in_loc = in_idx % LTI_FPGA_SIG_LEN;
	
	// Calculates output buffer location corresponting to current DSP input buffer location	
	// The buffers are started simultaneously 
	out_loc= (in_idx+gen_delay) % LTI_FPGA_SIG_LEN;
	
	
	
	// ADD DSP CODE HERE	
	
	// IIR FILTER implementation
	
	
/**	
**                      +--------------+      FIR             IIR                                                  
**  INPUT               |              |                                                            OUTPUT             
**+----------+---------->   x b0 (p0)  +----->(+)------------>(+)---------------------------------+--------->
**           |          |              |       ^               ^                                  |          
**           |          +--------------+       |               |                                  |          
**           |                                 |               |                                  |          
**           |                                 |               |                                  |    
**    +------v------+                          |               |                           +------v------+   
**    |             |                          |               |                           |             |   
**    |     s0      |                          |               |                           |     s64     |   
**    |             |                          |               |                           |             |   
**    +------+------+                          |               |                           +------+------+                 
**           |          +--------------+       |               |        +----------------+        |          
**           |          |              |       |               |        |                |        |          
**           +---------->   x b1 (p1)  +----->(+)             (+)<------+  x -a1 (-p65)  <--------+          
**           |          |              |       ^               ^        |                |        |          
**           |          +--------------+       |               |        +----------------+        |          
**    +------v------+                          |               |                           +------v------+   
**    |             |                          |               |                           |             |   
**    |     s1      |                          |               |                           |     s65     |   
**    |             |                          |               |                           |             |   
**    +------+------+                          |               |                           +------+------+   
**           |                                 |               |                                  |          
**           |          +--------------+       |               |        +----------------+        |          
**           |          |              |       |               |        |                |        |          
**           +---------->   x b2 (p2)  +----->(+)             (+)<------+  x -a2 (-p66)  <--------+          
**           |          |              |       ^               ^        |                |        |          
**           |          +--------------+       |               |        +----------------+        |          
**           v                                 +               +                                  v          
**                                                                                                           */

// MAC (MultiplyAndAccumulate) 
	
	// Retrieve input signal
	curr_in=conv_s14_to_double(g_lti_fpga_cha_mem[in_loc]);
	
	//First FIR coefficient (b0)
	fir=curr_in*cha_dsp_par[0];
	//Initialize IIR
	iir=0;
	
	// For every FIR and IIR coeff.
	for (dsp_ix=0; dsp_ix<(order-1); dsp_ix++)
	{
	  // Accumulate delayed FIR input times FIR coeffs. (starting from s0*b1 (at loc 1))
	  fir+=cha_state[dsp_ix]*cha_dsp_par[dsp_ix+1];
	  // Accumulate delayes IIR output state times IIR coeffs. (starting from s64*(-a1) (at loc p65), a0 is by definition =1, (at loc. p64 is stored filter order))
	  iir+=cha_state[64+dsp_ix]*(-cha_dsp_par[64+dsp_ix+1]);
	   
	  //fprintf(fp3, "  dsp_ix: %d (in_idx: %d) fir: %4.4f iir: %4.4f ParFIR: %4.4f ParIIR: %4.4f  stat_fir: %4.4f  stat_iir: %4.4f\n", dsp_ix, in_idx, fir, iir, cha_dsp_par[dsp_ix+1], (-cha_dsp_par[64+dsp_ix+1]), cha_state[dsp_ix], cha_state[64+dsp_ix] );   
	 
	}
	//add also FIR term
	iir+=fir;
	
	// Propagating states for next iteration: from last register in the chain to the first
	
	for (dsp_ix=(order-1); dsp_ix>0 ;dsp_ix--)
	{
	  // FIR states
	  cha_state[dsp_ix]=cha_state[dsp_ix-1];
	  // IIR states
	  cha_state[64+dsp_ix]=cha_state[64+dsp_ix-1];
	}
	//Updating first registers
	cha_state[0]=curr_in;
	cha_state[64]=iir;
	
	// First IIR register corresponds to output to be applied to DAC
	cha_awg[out_loc] = conv_double_to_s14(cha_state[64]);
	
	
	//IIR implementation ends
	
	

	// Signal acquisition: pass processed samples for GUI visualization purposes
	cha_in[out_idx] = g_lti_fpga_cha_mem[in_loc];
        chb_in[out_idx] = g_lti_fpga_chb_mem[in_loc];
	
      }
      // return next DSP cycle pointer
      dsp_ptr=(curr_ptr-1) % LTI_FPGA_SIG_LEN;;
      
      //Thid pointer can be used to calculate elapsed DSP time
      //lti_fpga_get_wr_ptr(&curr_ptr, NULL);
 
      
      
      
  return dsp_ptr;
}

double conv_s14_to_double(int sigs14)
{
         if(sigs14 >= (1<<13))
             return  ((double)sigs14 - (double)(1<<14));
         else
	     return (double) sigs14;
}   


int conv_double_to_s14(double sig)
{
  double mxs14=(double)((1<<13)-1);
  double mns14=(double)(-(1<<13));
  
  
         if(sig>mxs14)
	   sig=mxs14;
	 if(sig<mns14)
	   sig=mns14;
	 
  
         if(sig < 0)
             return  round(sig + (double)(1<<14));
         else
	     return round(sig);
} 



int lti_fpga_get_live_signal(double **cha_signal, double **chb_signal)
{
    int curr_ptr;
    int in_idx, out_idx;
    double *cha_o = *cha_signal;
    double *chb_o = *chb_signal;

    if(!cha_o || !chb_o) {
        fprintf(stderr, "lti_fpga_get_signal() not initialized\n");
        return -1;
    }

    // Check current pointer (the buffer is running ("live" mode) )
    
    // Use the current pointer (in place of trigger pointer) as circular buffer start reading point
    lti_fpga_get_wr_ptr(&curr_ptr, NULL);

    // Start 20 samples after current pointer to avoid RW conflicts 
    //(experimentally this function takes 15 samples at 0.5 ms rate (about 8 ms) to read all data out)
         
    for(in_idx = (curr_ptr+20) + 1, out_idx = 0; 
        out_idx < (LTI_FPGA_SIG_LEN); in_idx++, out_idx++) {
      
        if(in_idx >= LTI_FPGA_SIG_LEN)
            in_idx = in_idx % LTI_FPGA_SIG_LEN;

        // At the middle of the buffer wait for the extra 20 samples to be written	
	if (out_idx==8000)
	{
	  usleep(200000); // much more than normally necessary...
	}
	  
	    
        cha_o[out_idx] = g_lti_fpga_cha_mem[in_idx];
        chb_o[out_idx] = g_lti_fpga_chb_mem[in_idx];

        // convert to signed
        if(cha_o[out_idx] > (double)(1<<13))
            cha_o[out_idx] -= (double)(1<<14);
        if(chb_o[out_idx] > (double)(1<<13))
            chb_o[out_idx] -= (double)(1<<14);
	
	
	
    }
    // Good to log curr_ptr_end for reading time measurement (without sleep: curr_ptr_end-curr_ptr=15)
    //lti_fpga_get_wr_ptr(&curr_ptr_end, NULL); 
   
    
    return 0;
}

int lti_fpga_get_wr_ptr(int *wr_ptr_curr, int *wr_ptr_trig)
{
    if(wr_ptr_curr)
        *wr_ptr_curr = g_lti_fpga_reg_mem->wr_ptr_cur;
    if(wr_ptr_trig)
        *wr_ptr_trig = g_lti_fpga_reg_mem->wr_ptr_trigger;

    return 0;
}
int lti_fpga_cnv_trig_source(int trig_imm, int trig_source, int trig_edge)
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

int lti_fpga_cnv_freq_range_to_dec(int freq_range)
{
    /* Input: 0, 1, 2, 3, 4, 5 translates to:
     * Output: 1x, 8x, 64x, 1kx, 8kx, 65kx */
    switch(freq_range) {
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

int lti_fpga_cnv_freq_range_to_unit(int freq_range)
{
    /* Input freq. range: 0, 1, 2, 3, 4, 5 translates to:
     * Output: 0 - [MHz], 1 - [kHz], 2 - [Hz] */
    switch(freq_range) {
    case 0:
    case 1:
        return 2;
        break;
    case 2:
    case 3:
    case 4:
        return 1;
        break;
    case 5:
        return 0;
        break;
    default:
        return -1;
        break;
    };

    return -1;
}

int lti_fpga_cnv_time_to_smpls(float time, int dec_factor)
{
    /* Calculate sampling period (including decimation) */
    float smpl_p = (c_lti_fpga_smpl_period * dec_factor);
    int fpga_smpls = (int)round(time / smpl_p);

    return fpga_smpls;
}

int lti_fpga_cnv_v_to_cnt(float voltage)
{
    int adc_cnts = 0;

    if((voltage > g_lti_fpga_adc_max_v) || (voltage < -g_lti_fpga_adc_max_v))
        return -1;
    
    adc_cnts = (int)round(voltage * (float)((int)(1<<c_lti_fpga_adc_bits)) / 
                          (2*g_lti_fpga_adc_max_v));

    /* Clip highest value (+14 is calculated in int32_t to 0x2000, but we have
     * only 14 bits 
     */
    if((voltage > 0) && (adc_cnts & (1<<(c_lti_fpga_adc_bits-1))))
        adc_cnts = (1<<(c_lti_fpga_adc_bits-1))-1;
    else
        adc_cnts = adc_cnts & ((1<<(c_lti_fpga_adc_bits))-1);

    return adc_cnts;
}

float lti_fpga_cnv_cnt_to_v(int cnts)
{
    int m;

    if(cnts & (1<<(c_lti_fpga_adc_bits-1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1<<c_lti_fpga_adc_bits)-1)) + 1);
    } else {
        m = cnts;
        /* positive number */
    }
    return (m * g_lti_fpga_adc_max_v /
            (float)(1<<(c_lti_fpga_adc_bits-1)));

}
