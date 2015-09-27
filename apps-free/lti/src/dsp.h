/**
 * $Id$
 *
 * @brief Red Pitaya LTI workbench DSP processing.
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

extern const double c_c2v;

/* Prepare frequency vector (output of size LTI_OUT_SIG_LEN) */
int rp_lti_prepare_freq_vector(float **freq_out, double f_s,
                                  float freq_range);

/* Processing stuff - Hanning window */
#define RP_LTI_HANN_AMP 0.8165 // Hann window power scaling (1/sqrt(sum(rcos.^2/N)))
int rp_lti_hann_init();
int rp_lti_hann_clean();

/* Input & Outputs of LTI_FPGA_SIG_LEN */
int rp_lti_hann_filter(double *cha_in, double *chb_in,
                          double **cha_out, double **chb_out);

int rp_lti_fft_init();
int rp_lti_fft_clean();

/* Inputs length: LTI_FPGA_SIG_LEN
 * Outputs length: floor(LTI_FPGA_SIG_LEN/2) 
 * Output is not complex number as usually is from the FFT but abs() value of the
 * calculation.
 */
int rp_lti_fft(double *cha_in, double *chb_in, 
                  double **cha_out, double **chb_out);


/*
 * Decimation (usually from internal 8k -> output 2k)
*/
int rp_lti_decimate(double *cha_in, double *chb_in,
                       float **cha_out, float **chb_out,
                       int in_len, int out_len);

/* Converts amplitude of the signal to Voltage (k_c2v - counts 2 voltage) and
 * to dBm (k_dBm) & convert to linear scale (20*log10())
 * Input & Outputs of length LTI_OUT_SIG_LEN (decimated length)
 */
int rp_lti_cnv_to_dBm(float *cha_in, float *chb_in,
                         float **cha_out, float **chb_out,
                         float *peak_power_cha, float *peak_freq_cha,
                         float *peak_power_chb, float *peak_freq_chb,
                         float freq_range);

int rp_lti_calc_fresp(float **cha_out, float **chb_out,
                         double **dsp_par_a, double **dsp_par_b,
                         float freq_range);


#endif //__DSP_H
