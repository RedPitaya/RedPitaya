/**
 * @brief Red Pitaya Frequency Response Analyzer DSC processing.
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
int rp_resp_prepare_freq_vector(float **freq_out, double f_s,
                                float freq_range, int II, int JJ, int k1, int kstp);

/* Calculate frequency response */
int rp_resp_calc(double *cha_in, double *chb_in, int k1, double scale, int kstp, int II,
                 double **cha_out, double **chb_out);

int rp_spectr_fft_init();
int rp_spectr_fft_clean();

int rp_resp_cnv_to_dB(double *cha_resp_in, double *chb_resp_in,
                      double *cha_resp_cal_in, double *chb_resp_cal_in,
                      float **cha_out, float **chb_out, int resp_len);

int rp_resp_init_sigs(float **freq_out, float **cha_out, float **chb_out);


#endif //__DSP_H
