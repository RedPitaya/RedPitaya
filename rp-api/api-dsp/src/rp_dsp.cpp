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

#include <new>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "rp_dsp.h"
#include "kiss_fftr.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// unsigned short      g_signal_fgpa_length = ADC_BUFFER_SIZE;
/* length of output signals: floor(SPECTR_FPGA_SIG_LEN/2) */

//#define SPECTR_OUT_SIG_LENGTH (rp_get_spectr_out_signal_length())
//#define SPECTR_FPGA_SIG_LEN   (rp_get_spectr_signal_length())

/* Const - [W] -> [mW] */
constexpr double g_w2mw = 1000;

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

using namespace rp_dsp_api;

auto __zeroethOrderBessel( double x ) -> double {
    const double eps = 0.000001;
    double Value = 0;
    double term = 1;
    double m = 0;

    while(term  > eps * Value){
        Value += term;
        ++m;
        term *= (x * x) / (4 * m * m);
    }   
    return Value;
}

struct CDSP::Impl {
    uint32_t m_max_adc_buffer_size;
    uint32_t m_signal_length;
    uint32_t m_adc_max_speed;
    uint8_t  m_max_channels;
    double   m_imp = 50;
    double   m_window_sum = 1;
    window_mode_t m_window_mode = HANNING;
    double*  m_window = NULL;
    bool     m_remove_DC = true;
    mode_t   m_mode = DBM;
    kiss_fft_cpx** m_kiss_fft_out = NULL;
    kiss_fftr_cfg  m_kiss_fft_cfg = NULL;
};


CDSP::CDSP(uint8_t max_channels,uint32_t max_adc_buffer,uint32_t adc_max_speed) {
    m_pimpl = new Impl();
    m_pimpl->m_max_adc_buffer_size = max_adc_buffer;
    m_pimpl->m_signal_length = max_adc_buffer;
    m_pimpl->m_adc_max_speed = adc_max_speed;
    m_pimpl->m_max_channels = max_channels;
    m_pimpl->m_imp = 50;
    m_pimpl->m_window_sum = 1;
    m_pimpl->m_window_mode = HANNING;
    m_pimpl->m_window = NULL;
    m_pimpl->m_remove_DC = true;
    m_pimpl->m_mode = DBM;
    m_pimpl->m_kiss_fft_out = NULL;
    m_pimpl->m_kiss_fft_cfg = NULL;
}

CDSP::~CDSP(){    
    fftClean();
    window_clean();
    delete m_pimpl;
}

auto CDSP::setImpedance(double value) -> void {
    if (value > 0)
        m_pimpl->m_imp = value;    
}

auto CDSP::getImpedance() -> double{
    return m_pimpl->m_imp;
}

auto CDSP::setSignalLength(uint32_t len) -> int {
    if (len < 256 || len > m_pimpl->m_max_adc_buffer_size) return -1;
    
    unsigned char res = 0;
    int n = len; 
    while (n) {
        res += n&1;
        n >>= 1;
    }
    if (res != 1) return -1;
    m_pimpl->m_signal_length = len;
    return 0;
}

auto CDSP::getOutSignalLength() -> uint32_t {
    return getSignalLength()/2;
}

auto CDSP::getOutSignalMaxLength() -> uint32_t {
    return getSignalMaxLength()/2;
}

auto CDSP::getSignalLength() -> uint32_t {
    return m_pimpl->m_signal_length;
}

auto CDSP::getSignalMaxLength() -> uint32_t {
    return m_pimpl->m_max_adc_buffer_size;
}


int CDSP::window_init(CDSP::window_mode_t mode){
    uint32_t i;
    m_pimpl->m_window_sum  = 0;
    m_pimpl->m_window_mode = mode;
    window_clean();
    
    try{
        m_pimpl->m_window = new double[getSignalMaxLength()];
    } catch (const std::bad_alloc& e) {
        fprintf(stderr, "rp_spectr_window_init() can not allocate mem\n");
        return -1;
    }
    
    switch(m_pimpl->m_window_mode) {
        case HANNING:{
            for(i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = RP_SPECTR_HANN_AMP * 
                (1 - cos(2*M_PI*i / (double)(getSignalLength()-1)));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case RECTANGULAR:{
           for(i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = 1;
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case HAMMING:{
            for(i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = 0.54 - 
                0.46 * cos(2*M_PI*i / (double)(getSignalLength()-1));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case BLACKMAN_HARRIS:{
            for(i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = RP_BLACKMAN_A0 - 
                               RP_BLACKMAN_A1 * cos(2*M_PI*i / (double)(getSignalLength()-1)) +
                               RP_BLACKMAN_A2 * cos(4*M_PI*i / (double)(getSignalLength()-1)) -
                               RP_BLACKMAN_A3 * cos(6*M_PI*i / (double)(getSignalLength()-1));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case FLAT_TOP:{
            for(i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = RP_FLATTOP_A0 - 
                               RP_FLATTOP_A1 * cos(2*M_PI*i / (double)(getSignalLength()-1)) +
                               RP_FLATTOP_A2 * cos(4*M_PI*i / (double)(getSignalLength()-1)) -
                               RP_FLATTOP_A3 * cos(6*M_PI*i / (double)(getSignalLength()-1)) +
                               RP_FLATTOP_A4 * cos(8*M_PI*i / (double)(getSignalLength()-1));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case KAISER_4:{
            const double x = 1.0 / __zeroethOrderBessel(4);
            const double y = (getSignalLength() - 1) / 2.0;

            for(i = 0; i < getSignalLength(); i++) {
                const double K = (i - y) / y;
                const double arg = sqrt( 1.0 - (K * K) );
                m_pimpl->m_window[i] = __zeroethOrderBessel( 4 * arg ) * x;
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }

        case KAISER_8:{
            const double x = 1.0 / __zeroethOrderBessel(8);
            const double y = (getSignalLength() - 1) / 2.0;

            for(i = 0; i < getSignalLength(); i++) {
                const double K = (i - y) / y;
                const double arg = sqrt( 1.0 - (K * K) );
                m_pimpl->m_window[i] = __zeroethOrderBessel( 8 * arg ) * x;
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        default:
            window_clean();
            return -1;
    }
    return 0;
}

auto CDSP::window_clean() -> int {
    delete m_pimpl->m_window;
    m_pimpl->m_window = NULL;
    return 0;
}

auto CDSP::getCurrentWindowMode() -> CDSP::window_mode_t {
    return m_pimpl->m_window_mode;
}

auto CDSP::setRemoveDC(bool enable) -> void {
    m_pimpl->m_remove_DC = enable;
}

auto CDSP::getRemoveDC() -> bool {
    return m_pimpl->m_remove_DC;
}

auto CDSP::setMode(CDSP::mode_t mode) -> void {
    m_pimpl->m_mode = mode;
}

auto CDSP::getMode() -> CDSP::mode_t {
    return m_pimpl->m_mode;
}


auto CDSP::prepareFreqVector(float **freq_out, double f_s, float decimation) -> int {
    uint32_t i;
    float *f = *freq_out;
    float freq_smpl = f_s / decimation;
    // (float)spectr_fpga_cnv_freq_range_to_dec(freq_range);
    /* Divider to get to the right units - [MHz], [kHz] or [Hz] */
    //float unit_div = 1e6;

    if(!f) {
        fprintf(stderr, "rp_spectr_prepare_freq_vector() not initialized\n");
        return -1;
    }
    
    for(i = 0; i < getOutSignalLength(); i++) {
        /* We use full FPGA signal length range for this calculation, eventhough
         * the output vector is smaller. */
        f[i] = (float)i / (float)getSignalLength() * freq_smpl;
    }

    return 0;
}


auto CDSP::windowFilter(double **_in,double ***_out) -> int {
    uint32_t i,j;
    double **ch_o = *_out;
    
    if(!_in || !_out)
        return -1;

    for(j = 0; j < m_pimpl->m_max_channels; j++) {
        for(i = 0; i < getSignalLength(); i++) {
            ch_o[j][i] = _in[j][i] * m_pimpl->m_window[i];
        }
    }
    return 0;
}

auto CDSP::fftInit() -> int {
    if(m_pimpl->m_kiss_fft_out || m_pimpl->m_kiss_fft_cfg) {
        fftClean();
    }

    m_pimpl->m_kiss_fft_out  = new kiss_fft_cpx*[m_pimpl->m_max_channels];
    for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        m_pimpl->m_kiss_fft_out[j] = new kiss_fft_cpx[getSignalLength()];
    }

    m_pimpl->m_kiss_fft_cfg = kiss_fftr_alloc(getSignalLength(), 0, NULL, NULL);
    return 0;
}


auto CDSP::fftClean() -> int {
    kiss_fft_cleanup();
    
    if(m_pimpl->m_kiss_fft_out) {    
        for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
            if (m_pimpl->m_kiss_fft_out[j])
                delete[] m_pimpl->m_kiss_fft_out[j];
        }
        delete[] m_pimpl->m_kiss_fft_out;
        m_pimpl->m_kiss_fft_out = NULL;

    }
    if(m_pimpl->m_kiss_fft_cfg) {
        free(m_pimpl->m_kiss_fft_cfg);
        m_pimpl->m_kiss_fft_cfg = NULL;
    }
    return 0;
}



auto CDSP::fft(double **_in, double ***_out) -> int {
    double **ch_o = *_out;
    if(!_in || !_out)
        return -1;

    if(!m_pimpl->m_kiss_fft_out  || !m_pimpl->m_kiss_fft_cfg) {
        fprintf(stderr, "rp_spect_fft not initialized");
        return -1;
    }

    for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        kiss_fftr(m_pimpl->m_kiss_fft_cfg, (kiss_fft_scalar *)_in[j], m_pimpl->m_kiss_fft_out[j]);
    }
    for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {                     // FFT limited to fs/2, specter of amplitudes
            ch_o[j][i] = sqrt(pow(m_pimpl->m_kiss_fft_out[j][i].r, 2) +
                            pow(m_pimpl->m_kiss_fft_out[j][i].i, 2));
        }
    }
    return 0;
}

auto CDSP::decimate(double **_in,float ***_out,uint32_t in_len, uint32_t out_len) -> int {
    uint32_t step;
    uint32_t i, j;
    float **ch_o = *_out;

    if(!_in || !_out)
        return -1;

    step = (uint32_t)round((float)in_len / (float)out_len);
    if(step < 1)
        step = 1;
    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        for(i = 0, j = 0; i < out_len; i++, j+=step) {
            uint32_t k=j;

            if(j >= in_len) {
                fprintf(stderr, "rp_spectr_decimate() index too high\n");
                return -1;
            }
            ch_o[c][i] = 0;

            for(k=j; k < j+step; k++) {
        
                double ch_p = 0;
                
                //dBm                
                if (getMode() == DBM){
                    /* Conversion to power (Watts) */
                    // V -> RMS -> power
                    ch_p = pow((_in[c][k] / m_pimpl->m_window_sum * 2) / 1.414213562, 2) /m_pimpl->m_imp;
                }
                // V
                if (getMode() == VOLT){
                    ch_p = _in[c][k] / m_pimpl->m_window_sum  * 2;
                }
                // dBu
                if (getMode() == DBU){
                    ch_p = _in[c][k] / m_pimpl->m_window_sum  * 2 / 1.414213562;
                }
        
                ch_o[c][i] += (double)ch_p;  // Summing the power expressed in Watts associated to each FFT bin
            }
            ch_o[c][i] /= step;
        }
    }
    return 0;
}

auto CDSP::cnvToDBM(float **_in,float ***_out,float **peak_power, float **peak_freq,uint32_t  decimation) -> int {
    float **ch_o = *_out;
    float *p_power = *peak_power;
    float *p_freq = *peak_freq;
    
    double *max_pw = new double[m_pimpl->m_max_channels];
    int *max_pw_idx = new int[m_pimpl->m_max_channels];
    float freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;
   
    if (m_pimpl->m_remove_DC) {
        for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
            ch_o[c][0] = ch_o[c][1] = ch_o[c][2];
        }
    }

    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        max_pw[c] =  -1e5;
        max_pw_idx[c] = 0;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */

            double ch_p=_in[c][i];
           
            // Avoiding -Inf due to log10(0.0)
            
            if (ch_p * g_w2mw > 1.0e-12 )	
                ch_o[c][i] = 10 * log10(ch_p * g_w2mw);  // W -> mW -> dBm
            else	
                ch_o[c][i] = 10 * log10(1.0e-12);  
            
            
            /* Find peaks */
            if(ch_o[c][i] > max_pw[c]) {
                max_pw[c]     = ch_o[c][i];
                max_pw_idx[c] = i;
            }
        }
        p_power[c] = max_pw[c];
        p_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * 
                        freq_smpl  / 2) ;
    }

    delete[] max_pw;
    delete[] max_pw_idx;

    return 0;
}

auto CDSP::cnvToDBMMaxValueRanged(float **_in,float ***_out,float **peak_power, float **peak_freq,uint32_t  decimation,uint32_t minFreq,uint32_t maxFreq) -> int {
    float **ch_o = *_out;
    float *p_power = (float*)peak_power;
    float *p_freq = (float*)peak_freq;
    
    double *max_pw  = new double[m_pimpl->m_max_channels];
    int *max_pw_idx = new int[m_pimpl->m_max_channels];
    float freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;
    if (m_pimpl->m_remove_DC) {
        for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
            ch_o[c][0] = ch_o[c][1] = ch_o[c][2];
        }
    }    
    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        max_pw[c] =  -1e5;
        max_pw_idx[c] = 0;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */

            double ch_p=_in[c][i];
           
            // Avoiding -Inf due to log10(0.0)
            
            if (ch_p * g_w2mw > 1.0e-12 )	
                ch_o[c][i] = 10 * log10(ch_p * g_w2mw);  // W -> mW -> dBm
            else	
                ch_o[c][i] = 10 * log10(1.0e-12);  
            
            auto currentFreq = ((float)i / (float)getOutSignalLength() * freq_smpl  / 2);
            if (currentFreq < minFreq || currentFreq > maxFreq) continue;
          
            /* Find peaks */
            if(ch_o[c][i] > max_pw[c]) {
                max_pw[c]     = ch_o[c][i];
                max_pw_idx[c] = i;
            }
        }
        p_power[c] = max_pw[c];
        p_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * 
                        freq_smpl  / 2) ;
    }
    delete[] max_pw;
    delete[] max_pw_idx;

    return 0;
}


auto CDSP::cnvToMetric(float **_in,float ***_out,float **peak_power, float **peak_freq,uint32_t  decimation) -> int{
    uint32_t i;
    float** ch_o = *_out;
    float *p_power = (float*)peak_power;
    float *p_freq =  (float*)peak_freq;
    
    double* max_pw = new double[m_pimpl->m_max_channels]; // -10000;
    int*    max_pw_idx = new int[m_pimpl->m_max_channels]; // 0;
    float  freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;

    if (m_pimpl->m_remove_DC) {
        for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
            ch_o[c][0] = ch_o[c][1] = ch_o[c][2];
        }
    }

    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        max_pw[c] = -10000;
        max_pw_idx[c] = 0;
        for(i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */
            if (getMode() == DBM){
                double ch_p=_in[c][i];
                if (ch_p * g_w2mw > 1.0e-12 )	
                    ch_o[c][i] = 10 * log10(ch_p * g_w2mw);  // W -> mW -> dBm
                else	
                    ch_o[c][i] = -120;            
            }

            if (getMode() == VOLT){
                ch_o[c][i] = _in[c][i];
            }

            if (getMode() == DBU){
                double ch_p=_in[c][i];
                // ( 20*log10( 0.686 / .775 ))
                if (ch_p * g_w2mw > 1.0e-12 )	
                    ch_o[c][i] = 20 * log10(ch_p / 0.775);
                else	
                    ch_o[c][i] = -120;            
            }

            /* Find peaks */
            if(ch_o[c][i] > max_pw[c]) {
                max_pw[c]     = ch_o[c][i];
                max_pw_idx[c] = i;
            }
        }


        p_power[c] = max_pw[c];
        p_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * 
                        freq_smpl  / 2) ;
    }

    delete[] max_pw;
    delete[] max_pw_idx;

    return 0;
}

