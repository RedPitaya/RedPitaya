/**
 * $Id$
 *
 * @brief Red Pitaya Power Analyzer DSC processing.
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
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "dsp.h"
#include "main.h"
#include "fpga.h"
#include "dsp.h"
#include "kiss_fftr.h"

extern float g_pwr_fpga_adc_max_v;
extern const int c_pwr_fpga_adc_bits;

/* length of output signals: floor(PWR_FPGA_SIG_LEN/2) */
const int c_dsp_sig_len = PWR_FPGA_SIG_LEN>>1;
const int pwr_dft_harmonic_num = 100;

/* Internal structures used in DSP  */
double            *rp_hann_window  = NULL;
kiss_fft_cpx      *rp_kiss_fft_out = NULL;
kiss_fftr_cfg      rp_kiss_fft_cfg = NULL;
double            *rp_dft_out_re_U = NULL;
double            *rp_dft_out_im_U = NULL;
double            *rp_dft_out_re_I = NULL;
double            *rp_dft_out_im_I = NULL; 

const double PI2 = 2 * M_PI;


int rp_pwr_hann_init(int length)
{
    int i;

    rp_pwr_hann_clean();

    rp_hann_window = (double *)malloc(length * sizeof(double));
    if(rp_hann_window == NULL) {
        fprintf(stderr, "rp_pwr_hann_create() can not allocate mem");
        return -1;
    }
    
    for(i = 0; i < length; i++) {
        rp_hann_window[i] = RP_PWR_HANN_AMP * 
            (1 - cos(PI2 * i / (double)(length-1)));
    }

    return 0;
}

int rp_pwr_hann_clean()
{
    if(rp_hann_window) {
        free(rp_hann_window);
        rp_hann_window = NULL;
    }
    return 0;
}


int rp_pwr_hann_filter(double *ch_in, double *ch_out, int length)
{
    int i;
    
    if(!ch_in || !ch_out)
        return -1;
    for(i = 0; i < length; i++) {
        ch_out[i] = ch_in[i] * rp_hann_window[i];
    }

    return 0;
}

int rp_pwr_fft_init(int length)
{
    if(rp_kiss_fft_out || rp_kiss_fft_cfg) {
        rp_pwr_fft_clean();
    }

    rp_kiss_fft_out = 
        (kiss_fft_cpx *)malloc(length * sizeof(kiss_fft_cpx));
    
    rp_kiss_fft_cfg = kiss_fftr_alloc(length, 0, NULL, NULL);

    return 0;
}

int rp_pwr_fft_clean()
{
    kiss_fft_cleanup();
    if(rp_kiss_fft_out) {
        free(rp_kiss_fft_out);
        rp_kiss_fft_out = NULL;
    }

    if(rp_kiss_fft_cfg) {
        free(rp_kiss_fft_cfg);
        rp_kiss_fft_cfg = NULL;
    }
    return 0;
}

int rp_pwr_fft(double *ch_in, double *max_amp_bin_1,
               double *max_amp_bin_2, double *max_amp_bin_3, 
               double *arg_max_bin, int *max_bin_num, int half_length)
{
    int i;
    double bin_amp = 0;
    double bin_max_amp = 0;
    int bin_num = 0;
    
    if(!ch_in)
        return -1;

    if(!rp_kiss_fft_out || !rp_kiss_fft_cfg) {
        fprintf(stderr, "rp_pwr_fft not initialized");
        return -1;
    }

    kiss_fftr(rp_kiss_fft_cfg, (kiss_fft_scalar *)ch_in, rp_kiss_fft_out);

    for(i = 0; i < half_length; i++) {                     // FFT limited to fs/2, specter of amplitudes        
      
        bin_amp = sqrt(pow(rp_kiss_fft_out[i].r, 2) + 
                        pow(rp_kiss_fft_out[i].i, 2));
                        
        if(bin_amp > bin_max_amp){
            bin_max_amp = bin_amp;
            bin_num = i;
        }        
    }
    
    if(bin_num == 0) {
     *max_amp_bin_1 = bin_max_amp;
     *max_amp_bin_2 = 0;
     *max_amp_bin_3 = 0;
     *max_bin_num = bin_num;
     *arg_max_bin = 0;
    
    } else if(bin_num == 1) {
     *max_amp_bin_1 = bin_max_amp;
     *max_amp_bin_2 = sqrt(pow(rp_kiss_fft_out[bin_num + 1].r, 2) + 
                          pow(rp_kiss_fft_out[bin_num + 1].i, 2));
     *max_amp_bin_3 = 0;
     *max_bin_num = bin_num;
     *arg_max_bin = atan2(rp_kiss_fft_out[bin_num].i, rp_kiss_fft_out[bin_num].r);
     
    } else {
     *max_amp_bin_1 = sqrt(pow(rp_kiss_fft_out[bin_num - 1].r, 2) + 
                         pow(rp_kiss_fft_out[bin_num - 1].i, 2));
                       
     *max_amp_bin_2 = bin_max_amp;                   
                        
     *max_amp_bin_3 = sqrt(pow(rp_kiss_fft_out[bin_num + 1].r, 2) + 
                          pow(rp_kiss_fft_out[bin_num + 1].i, 2));
                        
     *arg_max_bin = atan2(rp_kiss_fft_out[bin_num].i, rp_kiss_fft_out[bin_num].r);
    
     *max_bin_num = bin_num; 
    }
      
    return 0;
}

int rp_pwr_dft_init()
{
    if(rp_dft_out_re_U || rp_dft_out_im_U || rp_dft_out_re_I || rp_dft_out_im_I) {
        rp_pwr_dft_clean();
    }

    rp_dft_out_re_U = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    rp_dft_out_im_U = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    rp_dft_out_re_I = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    rp_dft_out_im_I = (double *)malloc(sizeof(double) * pwr_dft_harmonic_num);
    
    return 0;
}

int rp_pwr_dft_clean()
{
    if(rp_dft_out_re_U) {
        free(rp_dft_out_re_U);
        rp_dft_out_re_U = NULL;
    }
    
    if(rp_dft_out_im_U) {
        free(rp_dft_out_im_U);
        rp_dft_out_im_U = NULL;
    }
       
    if(rp_dft_out_re_I) {
        free(rp_dft_out_re_I);
        rp_dft_out_re_I = NULL;
    }
    
    if(rp_dft_out_im_I) {
        free(rp_dft_out_im_I);
        rp_dft_out_im_I = NULL;
    }
    
    return 0;
}

int rp_pwr_dft(double *cha_in, double *chb_in, int length, float rel_freq, 
               double *amp_U, double *amp_I, double *fi_U, double *fi_I)
{
    int k;
    int n;
    double K;
    double angle = 0;
    double sum_re_U = 0;
    double sum_im_U = 0;
    double sum_re_I = 0;
    double sum_im_I = 0; 
         
    if(!cha_in || !chb_in ||  !*amp_U ||  !*amp_I || !*fi_U || !*fi_I)
         return -1;

    if(!rp_dft_out_re_U || !rp_dft_out_im_U || !rp_dft_out_re_I || !rp_dft_out_im_I) {
         fprintf(stderr, "rp_pwr_dft not initialized");
         return -1;
    }

    for(k = 0; k < pwr_dft_harmonic_num; k++ ) {
     
        sum_re_U = 0;
        sum_im_U = 0;
        sum_re_I = 0;
        sum_im_I = 0; 
         
         K = (k + 1) * PI2 * rel_freq / length;	

         for(n = 0; n < length; n++) { 
             
             angle = n * K;
             
             sum_re_U += cha_in[n] * cos(angle);
             sum_im_U += -cha_in[n] * sin(angle);
             sum_re_I += chb_in[n] * cos(angle);
             sum_im_I += -chb_in[n] * sin(angle);
         }

         rp_dft_out_re_U[k] = sum_re_U;
         rp_dft_out_im_U[k] = sum_im_U;
         rp_dft_out_re_I[k] = sum_re_I;
         rp_dft_out_im_I[k] = sum_im_I;

         amp_U[k] = sqrt(pow(rp_dft_out_re_U[k], 2) + 
                         pow(rp_dft_out_im_U[k], 2)) *
                    2 / length;
         amp_I[k] = sqrt(pow(rp_dft_out_re_I[k], 2) + 
                         pow(rp_dft_out_im_I[k], 2)) *
                    2 / length;
         fi_U[k] = atan2(rp_dft_out_im_U[k], rp_dft_out_re_U[k]);
         fi_I[k] = atan2(rp_dft_out_im_I[k], rp_dft_out_re_I[k]);

        }
     
     return 0;
}     

double rp_pwr_calc_d(double max_amp_bin_1, double max_amp_bin_2, 
                     double max_amp_bin_3)
{
	double d = 0;
	
	d = 2 * (fabs(max_amp_bin_3) - fabs(max_amp_bin_1)) / 
	     (fabs(max_amp_bin_1) + 2* fabs(max_amp_bin_2) + fabs(max_amp_bin_3));
	     
     return d;
}

double rp_pwr_calc_interpolated_amp(double max_amp_bin_1, double max_amp_bin_2, 
                                    double max_amp_bin_3, double dm)
{
     double am = 0;
     am = M_PI * dm * (1 - pow(dm, 2)) * (4 - pow(dm, 2)) *
          (fabs(max_amp_bin_1) + 2*fabs(max_amp_bin_2) + fabs(max_amp_bin_3)) /
          sin(M_PI * dm) / 3;
 
     return am;
}

