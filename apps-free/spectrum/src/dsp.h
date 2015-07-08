/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer DSC processing.
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

/* Prepare frequency vector (output of size SPECTR_OUT_SIG_LEN) */
int rp_spectr_prepare_freq_vector(float **freq_out, double f_s,
                                  float freq_range);

/* Processing stuff - Hanning window */
#define RP_SPECTR_HANN_AMP 0.8165 // Hann window power scaling (1/sqrt(sum(rcos.^2/N)))
int rp_spectr_hann_init();
int rp_spectr_hann_clean();

/* Input & Outputs of SPECTR_FPGA_SIG_LEN */
int rp_spectr_hann_filter(double *cha_in, double *chb_in,
                          double **cha_out, double **chb_out);

int rp_spectr_fft_init();
int rp_spectr_fft_clean();

/* Inputs length: SPECTR_FPGA_SIG_LEN
 * Outputs length: floor(SPECTR_FPGA_SIG_LEN/2) 
 * Output is not complex number as usually is from the FFT but abs() value of the
 * calculation.
 */
int rp_spectr_fft(double *cha_in, double *chb_in, 
                  double **cha_out, double **chb_out);


/*
 * Decimation (usually from internal 8k -> output 2k)
*/
int rp_spectr_decimate(double *cha_in, double *chb_in,
                       float **cha_out, float **chb_out,
                       int in_len, int out_len);

/* Converts amplitude of the signal to Voltage (k_c2v - counts 2 voltage) and
 * to dBm (k_dBm) & convert to linear scale (20*log10())
 * Input & Outputs of length SPECTR_OUT_SIG_LEN (decimated length)
 */
int rp_spectr_cnv_to_dBm(float *cha_in, float *chb_in,
                         float **cha_out, float **chb_out,
                         float *peak_power_cha, float *peak_freq_cha,
                         float *peak_power_chb, float *peak_freq_chb,
                         float freq_range);

#endif //__DSP_H
