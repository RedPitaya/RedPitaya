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

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "spec_dsp.h"
#include "kiss_fftr.h"
#include "rp_cross.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

unsigned short      g_signal_fgpa_length = ADC_BUFFER_SIZE;
/* length of output signals: floor(SPECTR_FPGA_SIG_LEN/2) */

#define SPECTR_OUT_SIG_LENGTH (rp_get_spectr_out_signal_length())
#define SPECTR_FPGA_SIG_LEN   (rp_get_spectr_signal_length())

#define RP_SPECTR_HANN_AMP 0.8165 // Hann window power scaling (1/sqrt(sum(rcos.^2/N)))

#define RP_BLACKMAN_A0 0.35875
#define RP_BLACKMAN_A1 0.48829
#define RP_BLACKMAN_A2 0.14128
#define RP_BLACKMAN_A3 0.01168

#define RP_FLATTOP_A0 0.21557895
#define RP_FLATTOP_A1 0.41663158
#define RP_FLATTOP_A2 0.277263158
#define RP_FLATTOP_A3 0.083578947
#define RP_FLATTOP_A4 0.006947368

/* Internal structures used in DSP  */
double               *rp_window   = NULL;
double                rp_window_sum = 1;
kiss_fft_cpx         *rp_kiss_fft_out1 = NULL;
kiss_fft_cpx         *rp_kiss_fft_out2 = NULL;
kiss_fftr_cfg         rp_kiss_fft_cfg  = NULL;


window_mode_t         g_window_mode = HANNING;
int                   g_mode = 0;
int                   g_remove_DC = 1;
/* constants - calibration dependant */
/* Power calc. impedance*/
double c_imp = 50;
/* Const - [W] -> [mW] */
const double c_w2mw = 1000;

int rp_spectr_prepare_freq_vector(float **freq_out, double f_s, float decimation)
{
    int i;
    float *f = *freq_out;
    float freq_smpl = f_s / decimation;
    // (float)spectr_fpga_cnv_freq_range_to_dec(freq_range);
    /* Divider to get to the right units - [MHz], [kHz] or [Hz] */
    float unit_div = 1e6;

    if(!f) {
        fprintf(stderr, "rp_spectr_prepare_freq_vector() not initialized\n");
        return -1;
    }
    
    if (freq_smpl > 1e3) {
        unit_div = 1e3;
    }

    if (freq_smpl > 1e6) {
        unit_div = 1;
    }

    for(i = 0; i < SPECTR_OUT_SIG_LENGTH; i++) {
        /* We use full FPGA signal length range for this calculation, eventhough
         * the output vector is smaller. */
        f[i] = (float)i / (float)SPECTR_FPGA_SIG_LEN * f  / unit_div;
    }

    return 0;
}

void   rp_set_impedance(double value){
    if (value > 0)
        c_imp = value;
}

double rp_get_impedance(){
    return c_imp;
}


unsigned short rp_get_spectr_out_signal_length(){
    return rp_get_spectr_signal_length()/2;
}

unsigned short rp_get_spectr_out_signal_max_length(){
    return rp_get_spectr_signal_max_length()/2;
}

unsigned short rp_get_spectr_signal_length(){
    return g_signal_fgpa_length;
}

unsigned short rp_get_spectr_signal_max_length(){
    return ADC_BUFFER_SIZE;
}

int rp_set_spectr_signal_length(int len){
    if (len < 256 || len > ADC_BUFFER_SIZE) return -1;
    
    unsigned char res = 0;
    int n = len; 
    while (n) {
        res += n&1;
        n >>= 1;
    }
    if (res != 1) return -1;

    g_signal_fgpa_length = len;
    return 0;
}

void rp_spectr_set_mode(int mode){
    g_mode = mode;
}

int rp_spectr_get_mode(){
    return g_mode;
}

double __zeroethOrderBessel( double x )
{
    const double eps = 0.000001;
    double Value = 0;
    double term = 1;
    double m = 0;

    while(term  > eps * Value)
    {
        Value += term;
        ++m;
        term *= (x*x) / (4*m*m);
    }   
    return Value;
}

int rp_spectr_window_init(window_mode_t mode){
    int i;
    rp_window_sum = 0;
    g_window_mode = mode;
    rp_spectr_window_clean();
    
    rp_window = (double *)malloc(rp_get_spectr_signal_max_length() * sizeof(double));
    if(rp_window == NULL) {
        fprintf(stderr, "rp_spectr_window_init() can not allocate mem");
        return -1;
    }

    switch(g_window_mode) {
        case HANNING:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = RP_SPECTR_HANN_AMP * 
                (1 - cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)));
                rp_window_sum += rp_window[i];
            }
            break;
        }
        case RECTANGULAR:{
           for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = 1;
                rp_window_sum += rp_window[i];
            }
            break;
        }
        case HAMMING:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = 0.54 - 
                0.46 * cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1));
                rp_window_sum += rp_window[i];
            }
            break;
        }
        case BLACKMAN_HARRIS:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = RP_BLACKMAN_A0 - 
                               RP_BLACKMAN_A1 * cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) +
                               RP_BLACKMAN_A2 * cos(4*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) -
                               RP_BLACKMAN_A3 * cos(6*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1));
                rp_window_sum += rp_window[i];
            }
            break;
        }
        case FLAT_TOP:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = RP_FLATTOP_A0 - 
                               RP_FLATTOP_A1 * cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) +
                               RP_FLATTOP_A2 * cos(4*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) -
                               RP_FLATTOP_A3 * cos(6*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) +
                               RP_FLATTOP_A4 * cos(8*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1));
                rp_window_sum += rp_window[i];
            }
            break;
        }
        case KAISER_4:{
            const double x = 1.0 / __zeroethOrderBessel(4);
            const double y = (SPECTR_FPGA_SIG_LEN - 1) / 2.0;

            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                const double K = (i - y) / y;
                const double arg = sqrt( 1.0 - (K * K) );
                rp_window[i] = __zeroethOrderBessel( 4 * arg ) * x;
                rp_window_sum += rp_window[i];
            }
            break;
        }

        case KAISER_8:{
            const double x = 1.0 / __zeroethOrderBessel(8);
            const double y = (SPECTR_FPGA_SIG_LEN - 1) / 2.0;

            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                const double K = (i - y) / y;
                const double arg = sqrt( 1.0 - (K * K) );
                rp_window[i] = __zeroethOrderBessel( 8 * arg ) * x;
                rp_window_sum += rp_window[i];
            }
            break;
        }
        default:
            rp_spectr_window_clean();
            return -1;
    }
    return 0;
}

window_mode_t rp_spectr_get_current_windows(){
    return g_window_mode;
}

void rp_spectr_remove_DC(int state){
    g_remove_DC = state;
}

int rp_spectr_get_remove_DC(){
    return g_remove_DC;
}

int rp_spectr_window_clean(){
    if(rp_window) {
        free(rp_window);
        rp_window = NULL;
    }
    return 0;
}

int rp_spectr_window_filter(double *cha_in, double *chb_in,
                          double **cha_out, double **chb_out){
    int i;
    double *cha_o = *cha_out;
    double *chb_o = *chb_out;

    if(!cha_in || !chb_in || !*cha_out || !*chb_out)
        return -1;

    for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
        cha_o[i] = cha_in[i] * rp_window[i];
        chb_o[i] = chb_in[i] * rp_window[i];
    }
    return 0;
}

int rp_spectr_fft_init()
{
    if(rp_kiss_fft_out1 || rp_kiss_fft_out2 || rp_kiss_fft_cfg) {
        rp_spectr_fft_clean();
    }

    rp_kiss_fft_out1 = 
        (kiss_fft_cpx *)malloc(rp_get_spectr_signal_length() * sizeof(kiss_fft_cpx));
    rp_kiss_fft_out2 =
        (kiss_fft_cpx *)malloc(rp_get_spectr_signal_length() * sizeof(kiss_fft_cpx));

    rp_kiss_fft_cfg = kiss_fftr_alloc(rp_get_spectr_signal_length(), 0, NULL, NULL);

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

int rp_spectr_fft(double *cha_in, double *chb_in, 
                  double **cha_out, double **chb_out)
{
    double *cha_o = *cha_out;
    double *chb_o = *chb_out;
    int i;
    if(!cha_in || !chb_in || !*cha_out || !*chb_out)
        return -1;

    if(!rp_kiss_fft_out1 || !rp_kiss_fft_out2 || !rp_kiss_fft_cfg) {
        fprintf(stderr, "rp_spect_fft not initialized");
        return -1;
    }

    kiss_fftr(rp_kiss_fft_cfg, (kiss_fft_scalar *)cha_in, rp_kiss_fft_out1);
    kiss_fftr(rp_kiss_fft_cfg, (kiss_fft_scalar *)chb_in, rp_kiss_fft_out2);
    for(i = 0; i < SPECTR_OUT_SIG_LENGTH; i++) {                     // FFT limited to fs/2, specter of amplitudes
        cha_o[i] = sqrt(pow(rp_kiss_fft_out1[i].r, 2) +
                        pow(rp_kiss_fft_out1[i].i, 2));
        chb_o[i] = sqrt(pow(rp_kiss_fft_out2[i].r, 2) +
                        pow(rp_kiss_fft_out2[i].i, 2));
    }
    return 0;
}

int rp_spectr_decimate(double *cha_in, double *chb_in, 
                       float **cha_out, float **chb_out,
                       int in_len, int out_len)
{
    int step;
    int i, j;
    float *cha_o = *cha_out;
    float *chb_o = *chb_out;

    if(!cha_in || !chb_in || !*cha_out || !*chb_out)
        return -1;

    step = (int)round((float)in_len / (float)out_len);
    if(step < 1)
        step = 1;
    for(i = 0, j = 0; i < out_len; i++, j+=step) {
        int k=j;

        if(j >= in_len) {
            fprintf(stderr, "rp_spectr_decimate() index too high\n");
            return -1;
        }
        cha_o[i] = 0;
        chb_o[i] = 0;
	

        for(k=j; k < j+step; k++) {
	
            double cha_p = 0;
            double chb_p = 0;
            //dBm
            if (rp_spectr_get_mode() == 0){
                /* Conversion to power (Watts) */
                // V -> RMS -> power
                cha_p = pow((cha_in[k] / rp_window_sum * 2) / 1.414213562, 2) / c_imp;
                chb_p = pow((chb_in[k] / rp_window_sum * 2) / 1.414213562, 2) / c_imp;
            }
            // V
            if (rp_spectr_get_mode() == 1){
                cha_p = cha_in[k] / rp_window_sum  * 2; 
                chb_p = chb_in[k] / rp_window_sum  * 2; 
            }
            // dBu
            if (rp_spectr_get_mode() == 2){
                cha_p = cha_in[k] / rp_window_sum  * 2 / 1.414213562;
                chb_p = chb_in[k] / rp_window_sum  * 2 / 1.414213562; 
            }

            
       
            cha_o[i] += (double)cha_p;  // Summing the power expressed in Watts associated to each FFT bin
            chb_o[i] += (double)chb_p;
        }
        cha_o[i] /= step;
        chb_o[i] /= step;
    }
    return 0;
}

int rp_spectr_cnv_to_dBm(float *cha_in, float *chb_in,
                         float **cha_out, float **chb_out,
                         float *peak_power_cha, float *peak_freq_cha,
                         float *peak_power_chb, float *peak_freq_chb,
                         float  decimation)
{
    int i;
    float *cha_o = *cha_out;
    float *chb_o = *chb_out;
    double max_pw_cha = -1e5;
    double max_pw_chb = -1e5;
    int max_pw_idx_cha = 0;
    int max_pw_idx_chb = 0;
    float freq_smpl = ADC_SAMPLE_RATE / decimation;
   
    if (g_remove_DC != 0) {
            cha_o[0] = cha_o[1] = cha_o[2];
            chb_o[0] = chb_o[1] = chb_o[2];
    }

    for(i = 0; i < SPECTR_OUT_SIG_LENGTH; i++) {

        /* Conversion to power (Watts) */

        double cha_p=cha_in[i];
        double chb_p=chb_in[i];    
        
        // Avoiding -Inf due to log10(0.0)
        
        if (cha_p * c_w2mw > 1.0e-12 )	
            cha_o[i] = 10 * log10(cha_p * c_w2mw);  // W -> mW -> dBm
        else	
            cha_o[i] = 10 * log10(1.0e-12);  
        
        
        if (chb_p * c_w2mw > 1.0e-12 )        	
            chb_o[i] = 10 * log10(chb_p * c_w2mw);
        else	
            chb_o[i] = 10 * log10(1.0e-12);
        
        /* Find peaks */
        if(cha_o[i] > max_pw_cha) {
            max_pw_cha     = cha_o[i];
            max_pw_idx_cha = i;
        }
        if(chb_o[i] > max_pw_chb) {
            max_pw_chb     = chb_o[i];
            max_pw_idx_chb = i;
        }
    }

       
    *peak_power_cha = max_pw_cha;
    *peak_freq_cha = ((float)max_pw_idx_cha / (float)SPECTR_OUT_SIG_LENGTH * 
                      freq_smpl  / 2) ;
    *peak_power_chb = max_pw_chb;
    *peak_freq_chb = ((float)max_pw_idx_chb / (float)SPECTR_OUT_SIG_LENGTH * 
                      freq_smpl / 2) ;

    return 0;
}


int rp_spectr_cnv_to_metric(float *cha_in, float *chb_in,
                            float **cha_out, float **chb_out,
                            float *peak_power_cha, float *peak_freq_cha,
                            float *peak_power_chb, float *peak_freq_chb,
                            float  decimation){
    int i;
    float *cha_o = *cha_out;
    float *chb_o = *chb_out;
    double max_pw_cha = 0;
    double max_pw_chb = 0;
    int    max_pw_idx_cha = 0;
    int    max_pw_idx_chb = 0;
    float  freq_smpl = ADC_SAMPLE_RATE / decimation;

    if (g_remove_DC != 0) {
            cha_o[0] = cha_o[1] = cha_o[2];
            chb_o[0] = chb_o[1] = chb_o[2];
    }

    for(i = 0; i < SPECTR_OUT_SIG_LENGTH; i++) {

        /* Conversion to power (Watts) */
        if (rp_spectr_get_mode() == 0){
            double cha_p=cha_in[i];
            double chb_p=chb_in[i];    
            if (cha_p * c_w2mw > 1.0e-12 )	
                cha_o[i] = 10 * log10(cha_p * c_w2mw);  // W -> mW -> dBm
            else	
                cha_o[i] = 10 * log10(1.0e-12);
        
            if (chb_p * c_w2mw > 1.0e-12 )        	
                chb_o[i] = 10 * log10(chb_p * c_w2mw);
            else	
                chb_o[i] = 10 * log10(1.0e-12);
        }

        if (rp_spectr_get_mode() == 1){
            cha_o[i] = cha_in[i];
            chb_o[i] = chb_in[i];
        }

        if (rp_spectr_get_mode() == 2){
            double cha_p=cha_in[i];
            double chb_p=chb_in[i];    
            // ( 20*log10( 0.686 / .775 ))
            if (cha_p * c_w2mw > 1.0e-12 )	
                cha_o[i] = 20 * log10(cha_p / 0.775);
            else	
                cha_o[i] = 20 * log10(1.0e-12);
        
            if (chb_p * c_w2mw > 1.0e-12 )
                chb_o[i] = 20 * log10(chb_p / 0.775);
            else	
                chb_o[i] = 20 * log10(1.0e-12);
        }

        /* Find peaks */
        if(cha_o[i] > max_pw_cha) {
            max_pw_cha     = cha_o[i];
            max_pw_idx_cha = i;
        }
        if(chb_o[i] > max_pw_chb) {
            max_pw_chb     = chb_o[i];
            max_pw_idx_chb = i;
        }
    }


    *peak_power_cha = max_pw_cha;
    *peak_freq_cha = ((float)max_pw_idx_cha / (float)SPECTR_OUT_SIG_LENGTH * 
                      freq_smpl  / 2) ;
    *peak_power_chb = max_pw_chb;
    *peak_freq_chb = ((float)max_pw_idx_chb / (float)SPECTR_OUT_SIG_LENGTH * 
                      freq_smpl / 2) ;

    return 0;
}

