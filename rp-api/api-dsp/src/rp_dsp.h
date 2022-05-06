/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer DSC processing.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_DSP_H__
#define __RP_DSP_H__

#include <stdint.h>

namespace rp_dsp_api{



class CDSP{

public:

    typedef enum{
        RECTANGULAR     = 0,
        HANNING         = 1,
        HAMMING         = 2,
        BLACKMAN_HARRIS = 3,
        FLAT_TOP        = 4,
        KAISER_4        = 5,
        KAISER_8        = 6
    } window_mode_t;

    typedef enum{
        DBM             = 0,
        VOLT            = 1,
        DBU             = 2
    } mode_t;

    CDSP(uint8_t max_channels,uint32_t adc_buffer,uint32_t adc_max_speed);
    ~CDSP();

    auto setChannel(uint8_t ch, bool enable) -> void;
    auto setSignalLength(uint32_t len) -> int;
    auto getSignalLength() -> uint32_t;
    auto getSignalMaxLength() -> uint32_t;
    auto getOutSignalLength() -> uint32_t;
    auto getOutSignalMaxLength() -> uint32_t;

    auto window_init(window_mode_t mode) -> int;
    auto window_clean() -> int;
    auto getCurrentWindowMode() -> CDSP::window_mode_t;

    auto setImpedance(double value) -> void;
    auto getImpedance() -> double;

    auto setRemoveDC(bool enable) -> void;
    auto getRemoveDC() -> bool;

    auto setMode(CDSP::mode_t mode) -> void;
    auto getMode() -> CDSP::mode_t;

    
    auto prepareFreqVector(float **freq_out, double f_s, float decimation) -> int;

    auto windowFilter(double **_in,double ***_out) -> int;
    auto fftInit() -> int;
    auto fftClean() -> int;
    auto fft(double **_in, double ***_out) -> int;
    auto decimate(double **_in,float ***_out,uint32_t in_len, uint32_t out_len) -> int;
    auto cnvToDBM(float **_in,float ***_out,float **peak_power, float **peak_freq,uint32_t  decimation) -> int;
    auto cnvToDBMMaxValueRanged(float **_in,float ***_out,float **peak_power, float **peak_freq,uint32_t  decimation,uint32_t minFreq,uint32_t maxFreq) -> int;
    auto cnvToMetric(float **_in,float ***_out,float **peak_power, float **peak_freq,uint32_t  decimation) -> int;

private:

    CDSP(const CDSP &) = delete;
    CDSP(CDSP &&) = delete;
    CDSP& operator=(const CDSP&) =delete;
    CDSP& operator=(const CDSP&&) =delete;

    struct Impl;
    // Pointer to the internal implementation
    Impl *m_pimpl;
};

}

// // extern const double c_c2v;

// /* Prepare frequency vector (output of size SPECTR_OUT_SIG_LEN) */
// int rp_spectr_prepare_freq_vector(float **freq_out, double f_s,
//                                   float freq_range);

// int rp_set_spectr_signal_length(int len);
// unsigned short rp_get_spectr_out_signal_length();
// unsigned short rp_get_spectr_out_signal_max_length();
// unsigned short rp_get_spectr_signal_length();
// unsigned short rp_get_spectr_signal_max_length();

//  int rp_spectr_window_init(window_mode_t mode);
//  int rp_spectr_window_clean();

// int rp_spectr_window_filter(double *cha_in, double *chb_in,
//                           double **cha_out, double **chb_out);
// window_mode_t rp_spectr_get_current_windows();

// void rp_spectr_remove_DC(int state);
// int rp_spectr_get_remove_DC();

// int rp_spectr_fft_init();
// int rp_spectr_fft_clean();

// void rp_spectr_set_mode(int mode);
// int  rp_spectr_get_mode();



// /* Inputs length: SPECTR_FPGA_SIG_LEN
//  * Outputs length: floor(SPECTR_FPGA_SIG_LEN/2) 
//  * Output is not complex number as usually is from the FFT but abs() value of the
//  * calculation.
//  */
// int rp_spectr_fft(double *cha_in, double *chb_in, 
//                   double **cha_out, double **chb_out);


// /*
//  * Decimation (usually from internal 8k -> output 2k)
// */
// int rp_spectr_decimate(double *cha_in, double *chb_in,
//                        float **cha_out, float **chb_out,
//                        int in_len, int out_len);

// /* Converts amplitude of the signal to Voltage (k_c2v - counts 2 voltage) and
//  * to dBm (k_dBm) & convert to linear scale (20*log10())
//  * Input & Outputs of length SPECTR_OUT_SIG_LEN (decimated length)
//  */
// int rp_spectr_cnv_to_dBm(float *cha_in, float *chb_in,
//                          float **cha_out, float **chb_out,
//                          float *peak_power_cha, float *peak_freq_cha,
//                          float *peak_power_chb, float *peak_freq_chb,
//                          float  decimation);

// int rp_spectr_cnv_to_metric(float *cha_in, float *chb_in,
//                          float **cha_out, float **chb_out,
//                          float *peak_power_cha, float *peak_freq_cha,
//                          float *peak_power_chb, float *peak_freq_chb,
//                          float  decimation);

// }


#endif
