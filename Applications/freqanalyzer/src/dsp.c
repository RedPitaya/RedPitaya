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


/* length of output signals: floor(SPECTR_FPGA_SIG_LEN/2) */
const int c_dsp_sig_len = SPECTR_FPGA_SIG_LEN / 2;

/* Internal structures used in DSP  */
double               *rp_hann_window   = NULL;
kiss_fft_cpx         *rp_kiss_fft_out1 = NULL;
kiss_fft_cpx         *rp_kiss_fft_out2 = NULL;
kiss_fftr_cfg         rp_kiss_fft_cfg  = NULL;

/* constants - calibration dependant */
/* Power calc. impedance*/
const double c_imp = 50;
/* Const - [W] -> [mW] */
const double c_w2mw = 1000;


int rp_resp_prepare_freq_vector(float **freq_out, double f_s,
                                float freq_range, int II, int JJ, int k1, int kstp)
{
    int i;
    float *f = *freq_out;

    float unit_div = 1e6;

    float freq_smpl = f_s / (float)spectr_fpga_cnv_freq_range_to_dec(freq_range);

    if(!f) {
        fprintf(stderr, "rp_spectr_prepare_freq_vector() not initialized\n");
        return -1;
    }

    switch(spectr_fpga_cnv_freq_range_to_unit(freq_range)) {
    case 2:
        unit_div = 1e6;
        break;
    case 1:
        unit_div = 1e3;
        break;
    case 0:
        unit_div = 1;
        break;
    default:
        fprintf(stderr, "rp_spectr_prepare_freq_vector() wrong freq_range\n");
        return -1;
    }

    for (i = 0;i < II*JJ; i++) {
        f[i] = ((float) (k1+ i *kstp )) * (float) freq_smpl / ((float) SPECTR_FPGA_SIG_LEN)/ unit_div;
    }

    if (II*JJ < SPECTR_OUT_SIG_LEN) {
        for (i = II*JJ; i < SPECTR_OUT_SIG_LEN; i++)
            f[i] = f[II*JJ - 1];
    }

    return 0;
}


int rp_resp_calc(double *cha_in, double *chb_in, int k1, double scale, int kstp, int II,
                 double **cha_out, double **chb_out)
{

    // FFT BASED IMPLEMENTATION
    double *cha_o = *cha_out;
    double *chb_o = *chb_out;

    int i;
    if(!cha_in || !chb_in ||  !*cha_out ||  !*chb_out )
        return -1;

    if(!rp_kiss_fft_out1 || !rp_kiss_fft_out2 || !rp_kiss_fft_cfg) {
        fprintf(stderr, "rp_spect_fft not initialized");
        return -1;
    }

    kiss_fftr(rp_kiss_fft_cfg, (kiss_fft_scalar *)cha_in, rp_kiss_fft_out1);
    kiss_fftr(rp_kiss_fft_cfg, (kiss_fft_scalar *)chb_in, rp_kiss_fft_out2);

    for(i = 0; i < II; i++) {

        cha_o[k1 + i] = sqrt(pow(rp_kiss_fft_out1[(k1 + i) * kstp].r, 2) +
                pow(rp_kiss_fft_out1[(k1 + i) * kstp].i, 2)) * scale;
        chb_o[k1 + i] = sqrt(pow(rp_kiss_fft_out2[(k1 + i) * kstp].r, 2) +
                pow(rp_kiss_fft_out2[(k1 + i) * kstp].i, 2)) * scale;

        /* Saturate to -200 dB */
        const double c_min_response = 1e-10;
        if (cha_o[k1 + i] < c_min_response)
            cha_o[k1 + i] = c_min_response;
        if (chb_o[k1 + i] < c_min_response)
            chb_o[k1 + i] = c_min_response;
    }

    return 0;
}


int rp_spectr_fft_init()
{
    if(rp_kiss_fft_out1 || rp_kiss_fft_out2 || rp_kiss_fft_cfg) {
        rp_spectr_fft_clean();
    }

    rp_kiss_fft_out1 = 
        (kiss_fft_cpx *)malloc(SPECTR_FPGA_SIG_LEN * sizeof(kiss_fft_cpx));
    rp_kiss_fft_out2 =
        (kiss_fft_cpx *)malloc(SPECTR_FPGA_SIG_LEN * sizeof(kiss_fft_cpx));

    rp_kiss_fft_cfg = kiss_fftr_alloc(SPECTR_FPGA_SIG_LEN, 0, NULL, NULL);

    return 0;
}


int rp_spectr_fft_clean()
{
    kiss_fft_cleanup();
    if(rp_kiss_fft_out1) {
        free(rp_kiss_fft_out1);
        rp_kiss_fft_out1 = NULL;
    }
    if(rp_kiss_fft_out2) {
        free(rp_kiss_fft_out2);
        rp_kiss_fft_out2 = NULL;
    }
    if(rp_kiss_fft_cfg) {
        free(rp_kiss_fft_cfg);
        rp_kiss_fft_cfg = NULL;
    }
    return 0;
}


int rp_resp_init_sigs(float **freq_out, float **cha_out, float **chb_out)
{
    int i;
    float *cha_o = *cha_out;
    float *chb_o = *chb_out;
    float *f = *freq_out;

    if( !*cha_out || !*chb_out || !*f)
        return -1;

    for(i = 0; i < SPECTR_OUT_SIG_LEN; i++) {
    	cha_o[i] = 0;
    	chb_o[i] = 0;
    	f[i] = (float)i/SPECTR_OUT_SIG_LEN * 60.0;
    }
    return 0;
}


int rp_resp_cnv_to_dB(double *cha_resp_in, double *chb_resp_in,
                      double *cha_resp_cal_in, double *chb_resp_cal_in,
                      float **cha_out, float **chb_out, int resp_len)
{
    int i;
    float *cha_o = *cha_out;
    float *chb_o = *chb_out;

    if(!cha_resp_in || !chb_resp_in || !cha_resp_cal_in || !chb_resp_cal_in)
        return -1;
    if(!cha_o || !chb_o)
        return -1;

    for(i = 0; i < SPECTR_OUT_SIG_LEN; i++) {

        if (i<resp_len) {

            if (cha_resp_in[i]/cha_resp_cal_in[i] > 1.0e-10)
                cha_o[i] = 20 * log10(cha_resp_in[i]/cha_resp_cal_in[i]);
            else
                cha_o[i] = -200.0;

            if (chb_resp_in[i]/chb_resp_cal_in[i] > 1.0e-10)
                chb_o[i] = 20 * log10(chb_resp_in[i]/chb_resp_cal_in[i]);
            else
                chb_o[i] = -200.0;

        } else {

            cha_o[i] = cha_o[resp_len-1];
            chb_o[i] = chb_o[resp_len-1];
        }
    }

    return 0;
}
