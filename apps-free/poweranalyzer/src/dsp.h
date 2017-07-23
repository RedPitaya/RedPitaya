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

#ifndef __DSP_H
#define __DSP_H

extern const int c_dsp_sig_len;

#define SQRT2 sqrt(2)

/* Processing stuff - Hanning window */
#define RP_PWR_HANN_AMP 0.50000 // 0.8165 Hann window power scaling (1/sqrt(sum(rcos.^2/N)))
int rp_pwr_hann_init(int length);
int rp_pwr_hann_clean(void);

/* Input & Outputs of PWR_FPGA_SIG_LEN */
int rp_pwr_hann_filter(double *ch_in, double *ch_out, int length);

int rp_pwr_fft_init(int length);
int rp_pwr_fft_clean(void);

int rp_pwr_fft(double *ch_in, double *max_amp_bin_1,
               double *max_amp_bin_2, double *max_amp_bin_3, 
               double *arg_max_bin, int *max_bin_num, int half_length);
               
int rp_pwr_dft_init(void);
int rp_pwr_dft_clean(void);

int rp_pwr_dft(double *cha_in, double *chb_in, int length, float rel_freq, 
               double *amp_U, double *amp_I, double *fi_U, double *fi_I);
               
double rp_pwr_calc_d(double max_amp_bin_1, double max_amp_bin_2, 
                     double max_amp_bin_3);
                     
double rp_pwr_calc_interpolated_amp(double max_amp_bin_1, double max_amp_bin_2, 
                                    double max_amp_bin_3, double dm);                                  

#endif // __DSP_H
