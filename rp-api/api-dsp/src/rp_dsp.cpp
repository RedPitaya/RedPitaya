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
#include <mutex>
#include <map>

#include "rp_dsp.h"
#include "kiss_fftr.h"

#include "rp_math.h"

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

template<typename T>
auto createArray(uint32_t count,uint32_t signalLen) -> T** {
    try{
        auto arr = new T*[count];
        for(uint32_t i = 0; i < count; i++){
            arr[i] = new T[signalLen];
        }
        return arr;
    }catch (const std::bad_alloc& e) {
        fprintf(stderr, "createArray() can not allocate mem\n");
        return nullptr;
    }
}

template<typename T>
auto deleteArray(uint32_t count,T **arr) -> void {
    if (!arr) return;
    for(uint32_t i = 0; i < count; i++){
        delete[] arr[i];
    }
    delete[] arr;
}

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
    std::mutex m_channelMutex;
    std::map<uint8_t,bool> m_channelState;
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
    for(uint8_t i = 0 ;i < max_channels; i++){
        m_pimpl->m_channelState[i] = true;
    }
}

CDSP::~CDSP(){    
    fftClean();
    window_clean();
    delete m_pimpl;
}

auto CDSP::setChannel(uint8_t ch, bool enable) -> void{
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    m_pimpl->m_channelState[ch] = enable;
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

auto CDSP::remoteDCCount() -> uint8_t{
    switch(m_pimpl->m_window_mode) {
        case HANNING:{
            return 2;
        }
        case RECTANGULAR:{
            return 2;
        }
        case HAMMING:{
            return 2;
        }
        case BLACKMAN_HARRIS:{
            return 5;
        }
        case FLAT_TOP:{
            return 5;
        }
        case KAISER_4:{
            return 5;
        }

        case KAISER_8:{
            return 4;
        }
        default:
            fprintf(stderr,"[Error] remoteDCCount: Unknown window window mode\n");
            return 0;
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


auto CDSP::prepareFreqVector(data_t *data, double f_s, float decimation) -> int {
    if (!data || !data->freq_vector){
        fprintf(stderr, "prepareFreqVector() data not initialized\n");
        return -1;
    }
    uint32_t i;
    float freq_smpl = f_s / decimation;

    // (float)spectr_fpga_cnv_freq_range_to_dec(freq_range);
    /* Divider to get to the right units - [MHz], [kHz] or [Hz] */
    //float unit_div = 1e6;
    
    for(i = 0; i < getOutSignalLength(); i++) {
        /* We use full FPGA signal length range for this calculation, eventhough
         * the output vector is smaller. */
        data->freq_vector[i] = (float)i / (float)getSignalLength() * freq_smpl;
    }

    return 0;
}


auto CDSP::windowFilter(data_t *data) -> int {
    uint32_t i,j;
    if (!data || !data->in || !data->filtred){
        fprintf(stderr, "windowFilter() data not initialized\n");
        return -1;
    }

    for(j = 0; j < m_pimpl->m_max_channels; j++) {
        for(i = 0; i < getSignalLength(); i++) {
            data->filtred[j][i] = data->in[j][i] * m_pimpl->m_window[i];
        }
    }
    data->is_data_filtred = true;
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



auto CDSP::fft(data_t *data) -> int {
    if (!data || !data->in || !data->filtred || !data->fft){
        fprintf(stderr, "fft() data not initialized\n");
        return -1;
    }

    if(!m_pimpl->m_kiss_fft_out  || !m_pimpl->m_kiss_fft_cfg) {
        fprintf(stderr, "rp_spect_fft not initialized");
        return -1;
    }

    auto _in = data->is_data_filtred ? data->filtred : data->in;
    for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        if (!m_pimpl->m_channelState[j]) continue;
        kiss_fftr(m_pimpl->m_kiss_fft_cfg, (kiss_fft_scalar *)_in[j], m_pimpl->m_kiss_fft_out[j]);
    }

    for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        if (!m_pimpl->m_channelState[j]) continue;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {                     // FFT limited to fs/2, specter of amplitudes            
            data->fft[j][i] = sqrtf_neon(pow(m_pimpl->m_kiss_fft_out[j][i].r, 2) +
                            pow(m_pimpl->m_kiss_fft_out[j][i].i, 2));
        }
    }
    return 0;
}

auto CDSP::decimate(data_t *data,uint32_t in_len, uint32_t out_len) -> int {
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    uint32_t step;
    uint32_t i, j;

    if (!data || !data->decimated || !data->fft){
        fprintf(stderr, "decimated() data not initialized\n");
        return -1;
    }

    step = (uint32_t)round((float)in_len / (float)out_len);
    if(step < 1)
        step = 1;
    
    float wsumf = 1.0 / (float)m_pimpl->m_window_sum * 2.0;

    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c]) continue; 
        for(i = 0, j = 0; i < out_len; i++, j+=step) {
            uint32_t k=j;

            if(j >= in_len) {
                fprintf(stderr, "rp_spectr_decimate() index too high\n");
                return -1;
            }
            data->decimated[c][i] = 0;

            for(k=j; k < j+step; k++) {
        
                double ch_p = 0;
                
                //dBm                
                if (getMode() == DBM){
                    /* Conversion to power (Watts) */
                    // V -> RMS -> power
                    ch_p = pow((data->fft[c][k] * wsumf) / 1.414213562, 2) /m_pimpl->m_imp;
                }
                // V
                if (getMode() == VOLT){
                    ch_p = data->fft[c][k] * wsumf;
                }
                // dBu
                if (getMode() == DBU){
                    ch_p = data->fft[c][k] * wsumf / 1.414213562;
                }
        
                data->decimated[c][i] += (double)ch_p;  // Summing the power expressed in Watts associated to each FFT bin
            }
            data->decimated[c][i] /= step;
        }
    }
    return 0;
}

auto CDSP::cnvToDBM(data_t *data,uint32_t  decimation) -> int {
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    if (!data || !data->decimated || !data->converted || !data->peak_freq || !data->peak_power ){
        fprintf(stderr, "cnvToDBM() data not initialized\n");
        return -1;
    }
    
    double *max_pw = new double[m_pimpl->m_max_channels];
    int *max_pw_idx = new int[m_pimpl->m_max_channels];

    float freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;
   
    if (m_pimpl->m_remove_DC) {
        int8_t count = remoteDCCount();
        for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
            for(int8_t x = 0 ; x < count; x++){
                data->decimated[c][x] = data->decimated[c][count];
            }
        }
    }

    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c]) continue;
        max_pw[c] =  -1e5;
        max_pw_idx[c] = 0;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */

            double ch_p = data->decimated[c][i];
           
            // Avoiding -Inf due to log10(0.0)
            
            if (ch_p * g_w2mw > 1.0e-12 )	
                data->converted[c][i] = 10 * log10f_neon(ch_p * g_w2mw);  // W -> mW -> dBm
            else	
                data->converted[c][i] = 10 * log10f_neon(1.0e-12);
            
            
            /* Find peaks */
            if(data->converted[c][i] > max_pw[c]) {
                max_pw[c]     = data->converted[c][i];
                max_pw_idx[c] = i;
            }
        }
        data->peak_power[c] = max_pw[c];
        data->peak_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * freq_smpl  / 2);
    }

    delete[] max_pw;
    delete[] max_pw_idx;

    return 0;
}

auto CDSP::cnvToDBMMaxValueRanged(data_t *data,uint32_t  decimation,uint32_t minFreq,uint32_t maxFreq) -> int {
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    if (!data || !data->decimated || !data->converted || !data->peak_freq || !data->peak_power ){
        fprintf(stderr, "cnvToDBM() data not initialized\n");
        return -1;
    }
    
    double *max_pw = new double[m_pimpl->m_max_channels];
    int *max_pw_idx = new int[m_pimpl->m_max_channels];
    float freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;
    if (m_pimpl->m_remove_DC) {
        int8_t count = remoteDCCount();
        for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
            for(int8_t x = 0 ; x < count; x++){
                data->decimated[c][x] = data->decimated[c][count];
            }
        }
    }  
    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c]) continue;
        max_pw[c] =  -1e5;
        max_pw_idx[c] = 0;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */

            double ch_p = data->decimated[c][i];
           
            // Avoiding -Inf due to log10(0.0)
            
            if (ch_p * g_w2mw > 1.0e-12 )	
                data->converted[c][i] = 10 * log10f_neon(ch_p * g_w2mw);  // W -> mW -> dBm
            else	
                data->converted[c][i] = 10 * log10f_neon(1.0e-12);
            
            auto currentFreq = ((float)i / (float)getOutSignalLength() * freq_smpl  / 2);
            if (currentFreq < minFreq || currentFreq > maxFreq) continue;
          
            /* Find peaks */
            if(data->converted[c][i] > max_pw[c]) {
                max_pw[c]     = data->converted[c][i];
                max_pw_idx[c] = i;
            }
        }
        data->peak_power[c] = max_pw[c];
        data->peak_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * freq_smpl  / 2) ;
    }
    delete[] max_pw;
    delete[] max_pw_idx;
    return 0;
}


auto CDSP::cnvToMetric(data_t *data,uint32_t  decimation) -> int{
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    if (!data || !data->decimated || !data->converted || !data->peak_freq || !data->peak_power ){
        fprintf(stderr, "cnvToDBM() data not initialized\n");
        return -1;
    }
    uint32_t i;
    
    double *max_pw = new double[m_pimpl->m_max_channels];
    int *max_pw_idx = new int[m_pimpl->m_max_channels];
    
    float  freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;

    if (m_pimpl->m_remove_DC) {
        int8_t count = remoteDCCount();
        for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
            for(int8_t x = 0 ; x < count; x++){
                data->decimated[c][x] = data->decimated[c][count];
            }
        }
    }

    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c]) continue;
        max_pw[c] = -10000;
        max_pw_idx[c] = 0;
        for(i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */
            if (getMode() == DBM){
                double ch_p=data->decimated[c][i];
                if (ch_p * g_w2mw > 1.0e-12 )	
                    data->converted[c][i] = 10 * log10f_neon(ch_p * g_w2mw);  // W -> mW -> dBm
                else	
                    data->converted[c][i] = -120;
            }

            if (getMode() == VOLT){
                data->converted[c][i] = data->decimated[c][i];
            }

            if (getMode() == DBU){
                double ch_p = data->decimated[c][i];
                // ( 20*log10( 0.686 / .775 ))
                if (ch_p * g_w2mw > 1.0e-12 )	
                    data->converted[c][i] = 20 * log10f_neon(ch_p / 0.775);
                else	
                    data->converted[c][i] = -120;
            }

            /* Find peaks */
            if(data->converted[c][i] > max_pw[c]) {
                max_pw[c]     = data->converted[c][i];
                max_pw_idx[c] = i;
            }
        }


        data->peak_power[c] = max_pw[c];
        data->peak_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * freq_smpl  / 2) ;
    }

    delete[] max_pw;
    delete[] max_pw_idx;

    return 0;
}

auto CDSP::createData() -> data_t *{
    data_t *d = nullptr;
    try{
        d = new data_t();
        d->in  = createArray<double>(m_pimpl->m_max_channels,getSignalMaxLength());
        d->filtred = createArray<double>(m_pimpl->m_max_channels,getSignalMaxLength());
        d->fft = createArray<double>(m_pimpl->m_max_channels,getSignalMaxLength());
        d->decimated = createArray<float>(m_pimpl->m_max_channels,getOutSignalMaxLength());
        d->converted = createArray<float>(m_pimpl->m_max_channels,getOutSignalMaxLength());
        d->peak_power = new float[m_pimpl->m_max_channels];
        d->peak_freq = new float[m_pimpl->m_max_channels];
        d->freq_vector = new float[getOutSignalMaxLength()];
        d->is_data_filtred = false;
        d->channels = m_pimpl->m_max_channels;
        return d;
    }catch (const std::bad_alloc& e) {
        deleteData(d);
        fprintf(stderr, "createArray() can not allocate mem\n");
        return nullptr;
    }
}

auto CDSP::deleteData(data_t *data) -> void{
    if (!data) return;
    deleteArray(data->channels,data->in);
    deleteArray(data->channels,data->fft);
    deleteArray(data->channels,data->filtred);
    deleteArray(data->channels,data->decimated);
    deleteArray(data->channels,data->converted);
    delete[] data->peak_freq;
    delete[] data->peak_power;
    delete[] data->freq_vector;
    delete data;
}

