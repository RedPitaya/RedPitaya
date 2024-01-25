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

#define __SHORT_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define FATAL(X)  {fprintf(stderr, "Error at line %d, file %s errno %d [%s] %s\n", __LINE__, __SHORT_FILENAME__, errno, strerror(errno),X); exit(1);}
#define WARNING(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[W] %s:%d %s\n",__SHORT_FILENAME__,__LINE__,error_msg);}

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
    if (len < 256 || len > m_pimpl->m_max_adc_buffer_size) {
        WARNING("Wrong buffer size %d",len)
        return -1;
    }

    unsigned char res = 0;
    int n = len;
    while (n) {
        res += n&1;
        n >>= 1;
    }
    if (res != 1) {
        WARNING("Wrong buffer size %d",len)
        return -1;
    }
    m_pimpl->m_signal_length = len;
    return 0;
}

int CDSP::setSignalLengthDiv2(uint32_t len){
    if (len > m_pimpl->m_max_adc_buffer_size) {
        WARNING("Buffer is too long %d",len)
        return -1;
    }
    len = (len / 2) * 2;
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


int CDSP::window_init(window_mode_t mode){
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

auto CDSP::getCurrentWindowMode() -> window_mode_t {
    return m_pimpl->m_window_mode;
}

auto CDSP::setRemoveDC(bool enable) -> void {
    m_pimpl->m_remove_DC = enable;
}

auto CDSP::getRemoveDC() -> bool {
    return m_pimpl->m_remove_DC;
}

auto CDSP::setMode(mode_t mode) -> void {
    m_pimpl->m_mode = mode;
}

auto CDSP::getMode() -> mode_t {
    return m_pimpl->m_mode;
}


auto CDSP::prepareFreqVector(data_t *data, double f_s, float decimation) -> int {
    if (!data || !data->m_freq_vector){
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
        data->m_freq_vector[i] = (float)i / (float)getSignalLength() * freq_smpl;
    }

    return 0;
}

auto CDSP::prepareFreqVector(data_t *data, float decimation) -> int{
    return prepareFreqVector(data,m_pimpl->m_adc_max_speed,decimation);
}

auto CDSP::windowFilter(data_t *data) -> int {
    uint32_t i,j;
    if (!data || !data->m_in || !data->m_filtred){
        fprintf(stderr, "windowFilter() data not initialized\n");
        return -1;
    }

    for(j = 0; j < m_pimpl->m_max_channels; j++) {
        for(i = 0; i < getSignalLength(); i++) {
            data->m_filtred[j][i] = data->m_in[j][i] * m_pimpl->m_window[i];
        }
    }
    data->m_is_data_filtred = true;
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
    if (!data || !data->m_in || !data->m_filtred || !data->m_fft){
        fprintf(stderr, "fft() data not initialized\n");
        return -1;
    }

    if(!m_pimpl->m_kiss_fft_out  || !m_pimpl->m_kiss_fft_cfg) {
        fprintf(stderr, "rp_spect_fft not initialized");
        return -1;
    }

    auto _in = data->m_is_data_filtred ? data->m_filtred : data->m_in;
    for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        if (!m_pimpl->m_channelState[j]) continue;
        kiss_fftr(m_pimpl->m_kiss_fft_cfg, (kiss_fft_scalar *)_in[j], m_pimpl->m_kiss_fft_out[j]);
    }

    for(uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        if (!m_pimpl->m_channelState[j]) continue;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {                     // FFT limited to fs/2, specter of amplitudes
            data->m_fft[j][i] = sqrtf_neon(pow(m_pimpl->m_kiss_fft_out[j][i].r, 2) +
                            pow(m_pimpl->m_kiss_fft_out[j][i].i, 2));
        }
    }
    return 0;
}

int CDSP::getAmpAndPhase(data_t *_data, double _freq, double *_amp1, double *_phase1, double *_amp2, double *_phase2){
    float wsumf = 1.0 / (float)m_pimpl->m_window_sum * 2.0;

    auto amp = [&](int ch, int i){
        return sqrtf(pow(m_pimpl->m_kiss_fft_out[ch][i].r, 2) +
                                pow(m_pimpl->m_kiss_fft_out[ch][i].i, 2)) * wsumf;
    };

    // for(uint32_t i = 0; i < getOutSignalLength() - 1; i++){
    //     if (_data->m_freq_vector[i] <= _freq && _freq <= _data->m_freq_vector[i + 1] ){

    //         auto t = (_freq - _data->m_freq_vector[i]) / (_data->m_freq_vector[i + 1]  - _data->m_freq_vector[i]);
    //         auto i00 = m_pimpl->m_kiss_fft_out[0][i].i;
    //         auto r00 = m_pimpl->m_kiss_fft_out[0][i].r;

    //         auto i10 = m_pimpl->m_kiss_fft_out[1][i].i;
    //         auto r10 = m_pimpl->m_kiss_fft_out[1][i].r;

    //         auto i01 = m_pimpl->m_kiss_fft_out[0][i + 1].i;
    //         auto r01 = m_pimpl->m_kiss_fft_out[0][i + 1].r;

    //         auto i11 = m_pimpl->m_kiss_fft_out[1][i + 1].i;
    //         auto r11 = m_pimpl->m_kiss_fft_out[1][i + 1].r;


    //         auto I0 = (i01 - i00) * t + i00;
    //         auto I1 = (i11 - i10) * t + i10;

    //         auto R0 = (r01 - r00) * t + r00;
    //         auto R1 = (r11 - r10) * t + r10;

    //         *_amp1 = sqrtf(pow(I0, 2) + pow(R0, 2)) * wsumf;
    //         *_amp2 = sqrtf(pow(I1, 2) + pow(R1, 2)) * wsumf;

    //         *_phase1 = atan2(I0,R0);
    //         *_phase2 = atan2(I1,R1);

    //         printf("i %d freq %f - %f t %f\n",i,_data->m_freq_vector[i],_data->m_freq_vector[i + 1],t);
    //         // printf("A-1 %f A0 %f A1 %f\n",amp(0,i-1),amp(0,i),amp(0,i+1));

    //         return 0;
    //     }
    // }

    for(uint32_t i = 0; i < getOutSignalLength(); i++){
        if (_data->m_freq_vector[i] >= _freq){
            *_amp1 = amp(0,i);
            *_amp2 = amp(1,i);
            *_phase1 = atan2(m_pimpl->m_kiss_fft_out[0][i].i,m_pimpl->m_kiss_fft_out[0][i].r);
            *_phase2 = atan2(m_pimpl->m_kiss_fft_out[1][i].i,m_pimpl->m_kiss_fft_out[1][i].r);
            return 0;
        }
    }

    return -1;
}


auto CDSP::decimate(data_t *data,uint32_t in_len, uint32_t out_len) -> int {
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    uint32_t step;
    uint32_t i, j;

    if (!data || !data->m_decimated || !data->m_fft){
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
            data->m_decimated[c][i] = 0;

            for(k=j; k < j+step; k++) {

                double ch_p = 0;

                //dBm
                if (getMode() == DBM){
                    /* Conversion to power (Watts) */
                    // V -> RMS -> power
                    ch_p = pow((data->m_fft[c][k] * wsumf) / 1.414213562, 2) /m_pimpl->m_imp;
                }
                // V
                if (getMode() == VOLT){
                    ch_p = data->m_fft[c][k] * wsumf;
                }
                // dBu
                if (getMode() == DBU){
                    ch_p = data->m_fft[c][k] * wsumf / 1.414213562;
                }

                // dBV
                // V -> RMS
                if (getMode() == DBV){
                    ch_p = data->m_fft[c][k] * wsumf  / 1.414213562;
                }

                 // dBuV
                if (getMode() == DBuV){
                    ch_p = data->m_fft[c][k] * wsumf / 1.414213562;
                }

                data->m_decimated[c][i] += (double)ch_p;  // Summing the power expressed in Watts associated to each FFT bin
            }
            data->m_decimated[c][i] /= step;
        }
    }
    return 0;
}

auto CDSP::cnvToDBM(data_t *data,uint32_t  decimation) -> int {
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    if (!data || !data->m_decimated || !data->m_converted || !data->m_peak_freq || !data->m_peak_power ){
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
                data->m_decimated[c][x] = data->m_decimated[c][count];
            }
        }
    }

    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c]) continue;
        max_pw[c] =  -1e5;
        max_pw_idx[c] = 0;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */

            double ch_p = data->m_decimated[c][i];

            // Avoiding -Inf due to log10(0.0)

            if (ch_p * g_w2mw > 1.0e-12 )
                data->m_converted[c][i] = 10 * log10f_neon(ch_p * g_w2mw);  // W -> mW -> dBm
            else
                data->m_converted[c][i] = 10 * log10f_neon(1.0e-12);


            /* Find peaks */
            if(data->m_converted[c][i] > max_pw[c]) {
                max_pw[c]     = data->m_converted[c][i];
                max_pw_idx[c] = i;
            }
        }
        data->m_peak_power[c] = max_pw[c];
        data->m_peak_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * freq_smpl  / 2);
    }

    delete[] max_pw;
    delete[] max_pw_idx;

    return 0;
}

auto CDSP::cnvToDBMMaxValueRanged(data_t *data,uint32_t  decimation,uint32_t minFreq,uint32_t maxFreq) -> int {
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    if (!data || !data->m_decimated || !data->m_converted || !data->m_peak_freq || !data->m_peak_power ){
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
                data->m_decimated[c][x] = data->m_decimated[c][count];
            }
        }
    }
    for(uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c]) continue;
        max_pw[c] =  -1e5;
        max_pw_idx[c] = 0;
        for(uint32_t i = 0; i < getOutSignalLength(); i++) {

            /* Conversion to power (Watts) */

            double ch_p = data->m_decimated[c][i];

            // Avoiding -Inf due to log10(0.0)

            if (ch_p * g_w2mw > 1.0e-12 )
                data->m_converted[c][i] = 10 * log10f_neon(ch_p * g_w2mw);  // W -> mW -> dBm
            else
                data->m_converted[c][i] = 10 * log10f_neon(1.0e-12);

            auto currentFreq = ((float)i / (float)getOutSignalLength() * freq_smpl  / 2);
            if (currentFreq < minFreq || currentFreq > maxFreq) continue;

            /* Find peaks */
            if(data->m_converted[c][i] > max_pw[c]) {
                max_pw[c]     = data->m_converted[c][i];
                max_pw_idx[c] = i;
            }
        }
        data->m_peak_power[c] = max_pw[c];
        data->m_peak_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * freq_smpl  / 2) ;
    }
    delete[] max_pw;
    delete[] max_pw_idx;
    return 0;
}


auto CDSP::cnvToMetric(data_t *data,uint32_t  decimation) -> int{
    std::lock_guard<std::mutex> lock(m_pimpl->m_channelMutex);
    if (!data || !data->m_decimated || !data->m_converted || !data->m_peak_freq || !data->m_peak_power ){
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
                data->m_decimated[c][x] = data->m_decimated[c][count];
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
                double ch_p=data->m_decimated[c][i];
                if (ch_p * g_w2mw > 1.0e-12 )
                    data->m_converted[c][i] = 10 * log10f_neon(ch_p * g_w2mw);  // W -> mW -> dBm
                else
                    data->m_converted[c][i] = -120;
            }

            if (getMode() == VOLT){
                data->m_converted[c][i] = data->m_decimated[c][i];
            }

            if (getMode() == DBU){
                double ch_p = data->m_decimated[c][i];
                // ( 20*log10( 0.686 / .775 ))
                if (ch_p * g_w2mw > 1.0e-12 )
                    data->m_converted[c][i] = 20 * log10f_neon(ch_p / 0.775);
                else
                    data->m_converted[c][i] = -120;
            }

            if (getMode() == DBV){
                double ch_p = data->m_decimated[c][i];
                // ( 20*log10( RMS / 1.0 ))
                if (ch_p * g_w2mw > 1.0e-12 )
                    data->m_converted[c][i] = 20 * log10f_neon(ch_p);
                else
                    data->m_converted[c][i] = -120;
            }

            if (getMode() == DBuV){
                  double ch_p = data->m_decimated[c][i];
                // ( 20*log10( RMS / 1.0 )) + 120
                if (ch_p * g_w2mw > 1.0e-12 )
                    data->m_converted[c][i] = 20 * log10f_neon(ch_p) + 120;
                else
                    data->m_converted[c][i] = -10;
            }

            /* Find peaks */
            if(data->m_converted[c][i] > max_pw[c]) {
                max_pw[c]     = data->m_converted[c][i];
                max_pw_idx[c] = i;
            }
        }


        data->m_peak_power[c] = max_pw[c];
        data->m_peak_freq[c] = ((float)max_pw_idx[c] / (float)getOutSignalLength() * freq_smpl  / 2) ;
    }

    delete[] max_pw;
    delete[] max_pw_idx;

    return 0;
}

auto CDSP::createData() -> data_t *{
    data_t *d = nullptr;
    try{
        d = new data_t();
        d->m_in  = createArray<double>(m_pimpl->m_max_channels,getSignalMaxLength());
        d->m_filtred = createArray<double>(m_pimpl->m_max_channels,getSignalMaxLength());
        d->m_fft = createArray<double>(m_pimpl->m_max_channels,getSignalMaxLength());
        d->m_decimated = createArray<float>(m_pimpl->m_max_channels,getOutSignalMaxLength());
        d->m_converted = createArray<float>(m_pimpl->m_max_channels,getOutSignalMaxLength());
        d->m_peak_power = new float[m_pimpl->m_max_channels];
        d->m_peak_freq = new float[m_pimpl->m_max_channels];
        d->m_freq_vector = new float[getOutSignalMaxLength()];
        d->m_is_data_filtred = false;
        d->m_channels = m_pimpl->m_max_channels;
        return d;
    }catch (const std::bad_alloc& e) {
        deleteData(d);
        fprintf(stderr, "createArray() can not allocate mem\n");
        return nullptr;
    }
}

auto CDSP::deleteData(data_t *data) -> void{
    if (!data) return;
    deleteArray(data->m_channels,data->m_in);
    deleteArray(data->m_channels,data->m_fft);
    deleteArray(data->m_channels,data->m_filtred);
    deleteArray(data->m_channels,data->m_decimated);
    deleteArray(data->m_channels,data->m_converted);
    delete[] data->m_peak_freq;
    delete[] data->m_peak_power;
    delete[] data->m_freq_vector;
    delete data;
}

