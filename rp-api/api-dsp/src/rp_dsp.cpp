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

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <map>
#include <mutex>
#include <new>
#include <type_traits>
#include <vector>

#include "rp_dsp.h"
#include "rp_log.h"

#include "kiss_fftr.h"

#include "rp_math.h"

#ifdef ARCH_ARM
#include <arm_neon.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// unsigned short      g_signal_fgpa_length = ADC_BUFFER_SIZE;
/* length of output signals: floor(SPECTR_FPGA_SIG_LEN/2) */

//#define SPECTR_OUT_SIG_LENGTH (rp_get_spectr_out_signal_length())
//#define SPECTR_FPGA_SIG_LEN   (rp_get_spectr_signal_length())

/* Const - [W] -> [mW] */
constexpr double g_w2mw = 1000;

#define LOG_LIMIT 1.0e-15
#define RP_SPECTR_HANN_AMP 0.8165  // Hann window power scaling (1/sqrt(sum(rcos.^2/N)))

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

auto __zeroethOrderBessel(double x) -> double {
    const double eps = 0.000001;
    double Value = 0;
    double term = 1;
    double m = 0;

    while (term > eps * Value) {
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
    uint8_t m_max_channels;
    double m_imp = 50;
    double m_window_sum = 1;
    window_mode_t m_window_mode = HANNING;
    std::vector<cdsp_data_t> m_window;
    bool m_remove_DC = true;
    std::vector<kiss_fft_cpx>* m_kiss_fft_out = NULL;
    kiss_fftr_cfg m_kiss_fft_cfg = NULL;
    size_t m_kiss_fft_cfg_max_size = 0;
    std::mutex m_channelMutex;
    std::map<uint8_t, bool> m_channelState;
    std::map<uint8_t, uint32_t> m_channelProbe;
    data_t* m_data = NULL;
};

CDSP::CDSP(uint8_t max_channels, uint32_t max_adc_buffer, uint32_t adc_max_speed, bool createStoredData) {
    m_pimpl = new Impl();
    m_pimpl->m_max_adc_buffer_size = max_adc_buffer;
    m_pimpl->m_signal_length = max_adc_buffer;
    m_pimpl->m_adc_max_speed = adc_max_speed;
    m_pimpl->m_max_channels = max_channels;
    m_pimpl->m_imp = 50;
    m_pimpl->m_window_sum = 1;
    m_pimpl->m_window_mode = HANNING;
    m_pimpl->m_remove_DC = true;
    m_pimpl->m_kiss_fft_cfg = NULL;
    m_pimpl->m_kiss_fft_out = new std::vector<kiss_fft_cpx>[max_channels];

    for (uint8_t i = 0; i < max_channels; i++) {
        m_pimpl->m_channelState[i] = true;
        m_pimpl->m_channelProbe[i] = 1;
        m_pimpl->m_kiss_fft_out[i].reserve(max_adc_buffer);
    }

    m_pimpl->m_window.reserve(max_adc_buffer);
    if (createStoredData) {
        m_pimpl->m_data = createData();
    }

    kiss_fftr_alloc(max_adc_buffer, 0, NULL, &m_pimpl->m_kiss_fft_cfg_max_size);  // precalculate memory
    if (m_pimpl->m_kiss_fft_cfg_max_size != 0) {
        uint8_t* memory = new uint8_t[m_pimpl->m_kiss_fft_cfg_max_size];
        m_pimpl->m_kiss_fft_cfg = kiss_fftr_alloc(max_adc_buffer, 0, (void*)memory, &m_pimpl->m_kiss_fft_cfg_max_size);
    }
}

CDSP::~CDSP() {
    if (m_pimpl->m_kiss_fft_out) {
        delete[] m_pimpl->m_kiss_fft_out;
        m_pimpl->m_kiss_fft_out = NULL;
    }

    kiss_fft_cleanup();

    if (m_pimpl->m_kiss_fft_cfg) {
        delete[] (uint8_t*)m_pimpl->m_kiss_fft_cfg;
        m_pimpl->m_kiss_fft_cfg = NULL;
    }

    delete m_pimpl;
}

auto CDSP::setChannel(uint8_t ch, bool enable) -> void {
    std::lock_guard lock(m_pimpl->m_channelMutex);
    m_pimpl->m_channelState[ch] = enable;
}

auto CDSP::getChannel(uint8_t ch, bool* enable) -> void {
    *enable = m_pimpl->m_channelState[ch];
}

auto CDSP::getStoredData() -> data_t* {
    return m_pimpl->m_data;
}

auto CDSP::setImpedance(double value) -> void {
    if (value > 0)
        m_pimpl->m_imp = value;
}

auto CDSP::getImpedance() -> double {
    return m_pimpl->m_imp;
}

auto CDSP::setSignalLength(uint32_t len) -> int {
    if (len < 256 || len > m_pimpl->m_max_adc_buffer_size) {
        WARNING("Wrong buffer size %d", len)
        return -1;
    }

    unsigned char res = 0;
    int n = len;
    while (n) {
        res += n & 1;
        n >>= 1;
    }
    if (res != 1) {
        WARNING("Wrong buffer size %d", len)
        return -1;
    }
    m_pimpl->m_signal_length = len;
    return 0;
}

int CDSP::setSignalLengthDiv2(uint32_t len) {
    if (len > m_pimpl->m_max_adc_buffer_size) {
        WARNING("Buffer is too long %d", len)
        return -1;
    }
    len = (len / 2) * 2;
    m_pimpl->m_signal_length = len;
    return 0;
}

auto CDSP::getOutSignalLength() -> uint32_t {
    return getSignalLength() / 2;
}

auto CDSP::getOutSignalMaxLength() -> uint32_t {
    return getSignalMaxLength() / 2;
}

auto CDSP::getSignalLength() -> uint32_t {
    return m_pimpl->m_signal_length;
}

auto CDSP::getSignalMaxLength() -> uint32_t {
    return m_pimpl->m_max_adc_buffer_size;
}

int CDSP::window_init(window_mode_t mode) {
    uint32_t i;
    m_pimpl->m_window_sum = 0;
    m_pimpl->m_window_mode = mode;

    try {
        auto size = getSignalMaxLength();
        m_pimpl->m_window.resize(size);
        if (m_pimpl->m_window.size() != size) {
            ERROR_LOG("Can not allocate memory");
            return -1;
        }
    } catch (const std::bad_alloc& e) {
        ERROR_LOG("Can not allocate memory");
        return -1;
    }

    switch (m_pimpl->m_window_mode) {
        case HANNING: {
            for (i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = RP_SPECTR_HANN_AMP * (1 - cos(2 * M_PI * i / (double)(getSignalLength() - 1)));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case RECTANGULAR: {
            for (i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = 1;
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case HAMMING: {
            for (i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = 0.54 - 0.46 * cos(2 * M_PI * i / (double)(getSignalLength() - 1));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case BLACKMAN_HARRIS: {
            for (i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = RP_BLACKMAN_A0 - RP_BLACKMAN_A1 * cos(2 * M_PI * i / (double)(getSignalLength() - 1)) +
                                       RP_BLACKMAN_A2 * cos(4 * M_PI * i / (double)(getSignalLength() - 1)) - RP_BLACKMAN_A3 * cos(6 * M_PI * i / (double)(getSignalLength() - 1));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case FLAT_TOP: {
            for (i = 0; i < getSignalLength(); i++) {
                m_pimpl->m_window[i] = RP_FLATTOP_A0 - RP_FLATTOP_A1 * cos(2 * M_PI * i / (double)(getSignalLength() - 1)) +
                                       RP_FLATTOP_A2 * cos(4 * M_PI * i / (double)(getSignalLength() - 1)) - RP_FLATTOP_A3 * cos(6 * M_PI * i / (double)(getSignalLength() - 1)) +
                                       RP_FLATTOP_A4 * cos(8 * M_PI * i / (double)(getSignalLength() - 1));
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        case KAISER_4: {
            const double x = 1.0 / __zeroethOrderBessel(4);
            const double y = (getSignalLength() - 1) / 2.0;

            for (i = 0; i < getSignalLength(); i++) {
                const double K = (i - y) / y;
                const double arg = sqrt(1.0 - (K * K));
                m_pimpl->m_window[i] = __zeroethOrderBessel(4 * arg) * x;
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }

        case KAISER_8: {
            const double x = 1.0 / __zeroethOrderBessel(8);
            const double y = (getSignalLength() - 1) / 2.0;

            for (i = 0; i < getSignalLength(); i++) {
                const double K = (i - y) / y;
                const double arg = sqrt(1.0 - (K * K));
                m_pimpl->m_window[i] = __zeroethOrderBessel(8 * arg) * x;
                m_pimpl->m_window_sum += m_pimpl->m_window[i];
            }
            break;
        }
        default:
            return -1;
    }
    return 0;
}

auto CDSP::remoteDCCount() -> uint8_t {
    switch (m_pimpl->m_window_mode) {
        case HANNING: {
            return 2;
        }
        case RECTANGULAR: {
            return 2;
        }
        case HAMMING: {
            return 2;
        }
        case BLACKMAN_HARRIS: {
            return 5;
        }
        case FLAT_TOP: {
            return 5;
        }
        case KAISER_4: {
            return 5;
        }

        case KAISER_8: {
            return 4;
        }
        default:
            ERROR_LOG("Unknown window window mode");
            return 0;
    }
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

auto CDSP::setProbe(uint8_t channel, uint32_t value) -> void {
    m_pimpl->m_channelProbe[channel] = value;
}

auto CDSP::getProbe(uint8_t channel, uint32_t* value) -> void {
    *value = m_pimpl->m_channelProbe[channel];
}

auto CDSP::prepareFreqVector(data_t* data, double f_s, float decimation) -> int {
    if (!data) {
        ERROR_LOG("Data not initialized");
        return -1;
    }
    uint32_t i;
    float freq_smpl = f_s / decimation;

    // (float)spectr_fpga_cnv_freq_range_to_dec(freq_range);
    /* Divider to get to the right units - [MHz], [kHz] or [Hz] */
    //float unit_div = 1e6;
    if (data->m_converted.m_maxFreq != (uint32_t)freq_smpl || data->m_converted.m_data_size != getOutSignalLength()) {
        for (i = 0; i < getOutSignalLength(); i++) {
            /* We use full FPGA signal length range for this calculation, eventhough
            * the output vector is smaller. */
            data->m_converted.m_freq_vector[i] = (float)i / (float)getSignalLength() * freq_smpl;
        }
    }
    data->m_converted.m_maxFreq = freq_smpl;
    data->m_converted.m_data_size = getOutSignalLength();
    return 0;
}

auto CDSP::prepareFreqVector(data_t* data, float decimation) -> int {
    return prepareFreqVector(data, m_pimpl->m_adc_max_speed, decimation);
}

auto CDSP::windowFilter(data_t* data) -> int {
    if (!data) {
        ERROR_LOG("Data not initialized");
        return -1;
    }
    for (uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        if (!m_pimpl->m_channelState[j])
            continue;
        // for (i = 0; i < getSignalLength(); i++) {
        //     data->m_filtred[j][i] = data->m_in[j][i] * m_pimpl->m_window[i] * m_pimpl->m_channelProbe[j];
        // }

        if constexpr (std::is_same_v<cdsp_data_t, float>) {
            multiply_arrays_neon(data->m_filtred[j].data(), data->m_in[j].data(), m_pimpl->m_window.data(), getSignalLength());
            multiply_array_by_scalar_float_neon(data->m_filtred[j].data(), data->m_filtred[j].data(), m_pimpl->m_channelProbe[j], getSignalLength());
        } else if constexpr (std::is_same_v<cdsp_data_t, double>) {
            // Cast to the expected type if your data structures can handle it
            multiply_arrays_double_neon(reinterpret_cast<double*>(data->m_filtred[j].data()),
                                        reinterpret_cast<const double*>(data->m_in[j].data()),
                                        reinterpret_cast<const double*>(m_pimpl->m_window.data()),
                                        getSignalLength());
            multiply_array_by_scalar_double_neon(reinterpret_cast<double*>(data->m_filtred[j].data()),
                                                 reinterpret_cast<double*>(data->m_filtred[j].data()),
                                                 static_cast<double>(m_pimpl->m_channelProbe[j]),
                                                 getSignalLength());
        }
    }
    data->m_is_data_filtred = true;
    return 0;
}

auto CDSP::fftInit() -> int {
    auto sigLen = getSignalLength();

    for (uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        m_pimpl->m_kiss_fft_out[j].resize(sigLen);
    }

    size_t size = m_pimpl->m_kiss_fft_cfg_max_size;
    m_pimpl->m_kiss_fft_cfg = kiss_fftr_alloc(sigLen, 0, m_pimpl->m_kiss_fft_cfg, &size);
    return 0;
}

auto CDSP::fft(data_t* data) -> int {

#ifdef ARCH_ARM
    auto neonMagnitudeOptimized = [](uint32_t length, kiss_fft_cpx* input, float* output) {
        const uint32_t neon_chunk = 8;

        uint32_t i = 0;
        for (; i + (neon_chunk - 1) < length; i += neon_chunk) {
            float32x4x2_t vec1 = vld2q_f32(&input[i].r);
            float32x4x2_t vec2 = vld2q_f32(&input[i + 4].r);

            float32x4_t real_sq1 = vmulq_f32(vec1.val[0], vec1.val[0]);
            float32x4_t imag_sq1 = vmulq_f32(vec1.val[1], vec1.val[1]);
            float32x4_t real_sq2 = vmulq_f32(vec2.val[0], vec2.val[0]);
            float32x4_t imag_sq2 = vmulq_f32(vec2.val[1], vec2.val[1]);

            float32x4_t sum_sq1 = vaddq_f32(real_sq1, imag_sq1);
            float32x4_t sum_sq2 = vaddq_f32(real_sq2, imag_sq2);

            // Simple reciprocal sqrt without refinement (faster but less accurate)
            float32x4_t rsqrt1 = vrsqrteq_f32(sum_sq1);
            float32x4_t rsqrt2 = vrsqrteq_f32(sum_sq2);

            float32x4_t mag1 = vmulq_f32(sum_sq1, rsqrt1);
            float32x4_t mag2 = vmulq_f32(sum_sq2, rsqrt2);

            vst1q_f32(&output[i], mag1);
            vst1q_f32(&output[i + 4], mag2);
        }

        for (; i < length; i++) {
            float real = input[i].r;
            float imag = input[i].i;
            output[i] = sqrtf(real * real + imag * imag);
        }
    };
#endif

    if (!data) {
        ERROR_LOG("Data not initialized");
        return -1;
    }

    if (!m_pimpl->m_kiss_fft_out || !m_pimpl->m_kiss_fft_cfg) {
        ERROR_LOG("rp_spect_fft not initialized");
        return -1;
    }
    auto& _in = data->m_is_data_filtred ? data->m_filtred : data->m_in;
    for (uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        if (!m_pimpl->m_channelState[j])
            continue;
        kiss_fftr(m_pimpl->m_kiss_fft_cfg, (kiss_fft_scalar*)_in[j].data(), m_pimpl->m_kiss_fft_out[j].data());
    }
    for (uint32_t j = 0; j < m_pimpl->m_max_channels; j++) {
        if (!m_pimpl->m_channelState[j])
            continue;
#ifdef ARCH_ARM
        neonMagnitudeOptimized(getOutSignalLength(), m_pimpl->m_kiss_fft_out[j].data(), data->m_fft[j].data());
#else
        for (uint32_t i = 0; i < getOutSignalLength(); i++) {  // FFT limited to fs/2, specter of amplitudes
            auto x = m_pimpl->m_kiss_fft_out[j][i].r * m_pimpl->m_kiss_fft_out[j][i].r;
            auto y = m_pimpl->m_kiss_fft_out[j][i].i * m_pimpl->m_kiss_fft_out[j][i].i;
            data->m_fft[j][i] = sqrtf(x + y);
        }
#endif
    }
    return 0;
}

int CDSP::getAmpAndPhase(data_t* _data, double _freq, double* _amp1, double* _phase1, double* _amp2, double* _phase2) {
    float wsumf = 1.0 / (float)m_pimpl->m_window_sum * 2.0;

    auto amp = [&](int ch, int i) {
        return sqrtf(pow(m_pimpl->m_kiss_fft_out[ch][i].r, 2) + pow(m_pimpl->m_kiss_fft_out[ch][i].i, 2)) * wsumf;
    };

    for (uint32_t i = 0; i < getOutSignalLength(); i++) {
        if (_data->m_converted.m_freq_vector[i] >= _freq) {
            *_amp1 = amp(0, i);
            *_amp2 = amp(1, i);
            *_phase1 = atan2(m_pimpl->m_kiss_fft_out[0][i].i, m_pimpl->m_kiss_fft_out[0][i].r);
            *_phase2 = atan2(m_pimpl->m_kiss_fft_out[1][i].i, m_pimpl->m_kiss_fft_out[1][i].r);
            return 0;
        }
    }

    return -1;
}

auto CDSP::decimate(data_t* data, uint32_t in_len, uint32_t out_len) -> int {
    if (in_len != out_len) {
        return decimate_ex(data, in_len, out_len);
    } else {
        return decimate_disabled(data, in_len);
    }
}

auto CDSP::decimate_ex(data_t* data, uint32_t in_len, uint32_t out_len) -> int {
    std::lock_guard lock(m_pimpl->m_channelMutex);

    if (!data) {
        ERROR_LOG("Data not initialized");
        return -1;
    }

    const uint32_t step = std::max(1u, static_cast<uint32_t>(std::round(static_cast<float>(in_len) / out_len)));
    const float wsumf = 2.0f / static_cast<float>(m_pimpl->m_window_sum);
    const float imp_reciprocal = 1.0f / static_cast<float>(m_pimpl->m_imp);
    const float step_f = static_cast<float>(step);

    const uint32_t max_j = (out_len - 1) * step;
    if (max_j + step > in_len) {
        ERROR_LOG("rp_spectr_decimate() index too high");
        return -1;
    }

    for (uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c])
            continue;
        auto& vec = data->m_dec_data_z[c];
        std::fill(vec.begin(), vec.end(), 0.0f);
        vec = data->m_dec_data_wsumf[c];
        std::fill(vec.begin(), vec.end(), 0.0f);
        vec = data->m_dec_data_scaled[c];
        std::fill(vec.begin(), vec.end(), 0.0f);
    }

    for (uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c])
            continue;

        auto& fft_channel = data->m_fft[c];
        auto& converted_z = data->m_dec_data_z[c];
        auto& converted_wsumf = data->m_dec_data_wsumf[c];
        auto& converted_scaled = data->m_dec_data_scaled[c];

        for (uint32_t i = 0, j = 0; i < out_len; i++, j += step) {
            float sum_z = 0.0f;
            float sum_fft_wsumf = 0.0f;
            float sum_fft_scaled = 0.0f;

            for (uint32_t k = j; k < j + step; k++) {
                const float fft_wsumf = fft_channel[k] * wsumf;
                const float fft_scaled = fft_wsumf * 0.707106781f;

                sum_fft_wsumf += fft_wsumf;
                sum_fft_scaled += fft_scaled;
                sum_z += (fft_scaled * fft_scaled) * imp_reciprocal;
            }
            converted_z[i] = sum_z / step_f;
            converted_wsumf[i] = sum_fft_wsumf / step_f;
            converted_scaled[i] = sum_fft_scaled / step_f;

            // converted_dbm[i] = sum_z / step_f;
            // converted_mw[i] = sum_z / step_f;
            // converted_dbw[i] = sum_z / step_f;
            // converted_volt[i] = sum_fft_wsumf / step_f;
            // converted_dbu[i] = sum_fft_scaled / step_f;
            // converted_dbv[i] = sum_fft_scaled / step_f;
            // converted_dbuV[i] = sum_fft_scaled / step_f;
        }
    }

    return 0;
}

auto CDSP::decimate_disabled(data_t* data, uint32_t in_len) -> int {
    std::lock_guard lock(m_pimpl->m_channelMutex);

    if (!data) {
        ERROR_LOG("Data not initialized");
        return -1;
    }

    const float wsumf = 2.0f / static_cast<float>(m_pimpl->m_window_sum);

    for (uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c])
            continue;

        auto& fft_data = data->m_fft[c];
        auto& converted_z = data->m_dec_data_z[c];
        auto& converted_wsumf = data->m_dec_data_wsumf[c];
        auto& converted_scaled = data->m_dec_data_scaled[c];

        const float imp_reciprocal = 1.0f / m_pimpl->m_imp;

        for (uint32_t i = 0; i < in_len; i++) {
            const cdsp_data_t fft_wsumf = fft_data[i] * wsumf;
            const cdsp_data_t fft_scaled = fft_wsumf * 0.707106781f;
            const cdsp_data_t z = (fft_scaled * fft_scaled) * imp_reciprocal;
            // dbm_vec[i] = z;
            // mw_vec[i] = z;
            // dbw_vec[i] = z;
            // volt_vec[i] = fft_wsumf;
            // dbu_vec[i] = fft_scaled;
            // dbv_vec[i] = fft_scaled;
            // dbuV_vec[i] = fft_scaled;

            converted_z[i] = z;
            converted_wsumf[i] = fft_wsumf;
            converted_scaled[i] = fft_scaled;
        }
    }

    return 0;
}

// auto CDSP::cnvToDBMMaxValueRanged(data_t* data, uint32_t decimation, uint32_t minFreq, uint32_t maxFreq) -> int {
//     std::lock_guard lock(m_pimpl->m_channelMutex);
//     if (!data) {
//         ERROR_LOG("Data not initialized");
//         return -1;
//     }

//     cdsp_data_t max_pw[COUNT_DSP_MODE];
//     int max_pw_idx[COUNT_DSP_MODE];

//     float freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;
//     float freq_const = freq_smpl / (2.0f * (float)getOutSignalLength());
//     auto logLim_10 = 10 * log10f_neon(LOG_LIMIT);
//     auto logLim_20 = 20 * log10f_neon(LOG_LIMIT);

//     if (m_pimpl->m_remove_DC) {
//         int8_t count = remoteDCCount();
//         for (uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
//             for (int8_t x = 0; x < count; x++) {
//                 data->m_dec_data_z[c][x] = data->m_dec_data_z[c][count];
//                 data->m_dec_data_wsumf[c][x] = data->m_dec_data_wsumf[c][count];
//                 data->m_dec_data_scaled[c][x] = data->m_dec_data_scaled[c][count];
//             }
//         }
//     }

//     for (uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
//         if (!m_pimpl->m_channelState[c])
//             continue;

//         for (int mode = MIN_DSP_MODE; mode < COUNT_DSP_MODE; mode++) {
//             max_pw[mode] = std::numeric_limits<cdsp_data_t>::lowest();
//             max_pw_idx[mode] = 0;
//         }

//         for (uint32_t i = 0; i < getOutSignalLength(); i++) {

//             auto& converted_z = data->m_dec_data_z[c];
//             auto& converted_wsumf = data->m_dec_data_wsumf[c];
//             auto& converted_scaled = data->m_dec_data_scaled[c];

//             auto& dbm_vec_c = data->m_converted.m_result[DBM][c];
//             auto& mw_vec_c = data->m_converted.m_result[MW][c];
//             auto& dbw_vec_c = data->m_converted.m_result[DBW][c];
//             auto& volt_vec_c = data->m_converted.m_result[VOLT][c];
//             auto& dbu_vec_c = data->m_converted.m_result[DBU][c];
//             auto& dbv_vec_c = data->m_converted.m_result[DBV][c];
//             auto& dbuV_vec_c = data->m_converted.m_result[DBuV][c];

//             /* Conversion to power (Watts) */
//             auto ch_p = converted_z[i] * g_w2mw;
//             dbm_vec_c[i] = ch_p > LOG_LIMIT ? 10 * log10f_neon(ch_p) : logLim_10;  // W -> mW -> dBm

//             volt_vec_c[i] = converted_wsumf[i];

//             // ( 20*log10( 0.686 / .775 ))
//             ch_p = converted_scaled[i] / 0.775;
//             dbu_vec_c[i] = ch_p > LOG_LIMIT ? 20 * log10f_neon(ch_p) : logLim_20;  // W -> mW -> dBm

//             // ( 20*log10( RMS / 1.0 ))
//             ch_p = converted_scaled[i];
//             dbv_vec_c[i] = ch_p > LOG_LIMIT ? 20 * log10f_neon(ch_p) : logLim_20;  // W -> mW -> dBm

//             // ( 20*log10( RMS / 1.0 )) + 120
//             ch_p = converted_scaled[i];
//             dbuV_vec_c[i] = ch_p > LOG_LIMIT ? 20 * log10f_neon(ch_p) + 120 : logLim_20 + 120;  // W -> mW -> dBm

//             // W -> mW
//             ch_p = converted_z[i] * g_w2mw;
//             mw_vec_c[i] = ch_p;

//             ch_p = converted_z[i];
//             dbw_vec_c[i] = ch_p > LOG_LIMIT ? 10 * log10f_neon(ch_p) : logLim_10;

//             auto currentFreq = ((float)i * freq_const);
//             if (currentFreq < minFreq || currentFreq > maxFreq)
//                 continue;

//             if (dbm_vec_c[i] >= max_pw[DBM]) {
//                 max_pw[DBM] = dbm_vec_c[i];
//                 max_pw_idx[DBM] = i;
//             }

//             if (volt_vec_c[i] >= max_pw[VOLT]) {
//                 max_pw[VOLT] = volt_vec_c[i];
//                 max_pw_idx[VOLT] = i;
//             }

//             if (dbu_vec_c[i] >= max_pw[DBU]) {
//                 max_pw[DBU] = dbu_vec_c[i];
//                 max_pw_idx[DBU] = i;
//             }

//             if (dbv_vec_c[i] >= max_pw[DBV]) {
//                 max_pw[DBV] = dbv_vec_c[i];
//                 max_pw_idx[DBV] = i;
//             }

//             if (dbuV_vec_c[i] >= max_pw[DBuV]) {
//                 max_pw[DBuV] = dbuV_vec_c[i];
//                 max_pw_idx[DBuV] = i;
//             }

//             if (mw_vec_c[i] >= max_pw[MW]) {
//                 max_pw[MW] = mw_vec_c[i];
//                 max_pw_idx[MW] = i;
//             }

//             if (dbw_vec_c[i] >= max_pw[DBW]) {
//                 max_pw[DBW] = dbw_vec_c[i];
//                 max_pw_idx[DBW] = i;
//             }
//         }
//         for (int mode = MIN_DSP_MODE; mode < COUNT_DSP_MODE; mode++) {
//             data->m_converted.m_peak_power[mode][c] = max_pw[mode];
//             data->m_converted.m_peak_freq[mode][c] = ((float)max_pw_idx[mode] / (float)getOutSignalLength() * freq_smpl / 2);
//         }
//     }
//     return 0;
// }

auto CDSP::cnvToMetric(data_t* data, uint32_t decimation, uint32_t minFreq, uint32_t maxFreq) -> int {
    std::lock_guard lock(m_pimpl->m_channelMutex);
    if (!data) {
        ERROR_LOG("Data not initialized");
        return -1;
    }

    bool skipPeakRange = minFreq == 0 && maxFreq == 0;

    cdsp_data_t max_pw[COUNT_DSP_MODE];
    int max_pw_idx[COUNT_DSP_MODE];
    float freq_smpl = (float)m_pimpl->m_adc_max_speed / (float)decimation;
    float freq_const = freq_smpl / (2.0f * (float)getOutSignalLength());
    static auto logLim_10 = 10.f * log10f(LOG_LIMIT);
    static auto logLim_20 = 20.f * log10f(LOG_LIMIT);
    static auto LOG_0775 = log10f(0.775);
    static auto LOG_W2MW = log10f(g_w2mw);

    auto isSkip = [skipPeakRange, freq_const, minFreq, maxFreq](uint32_t i) {
        if (skipPeakRange)
            return false;
        auto currentFreq = ((float)i * freq_const);
        if (currentFreq < minFreq || currentFreq > maxFreq)
            return true;
        return false;
    };

    if (m_pimpl->m_remove_DC) {
        int8_t count = remoteDCCount();
        for (uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
            for (int8_t x = 0; x < count; x++) {
                data->m_dec_data_z[c][x] = data->m_dec_data_z[c][count];
                data->m_dec_data_wsumf[c][x] = data->m_dec_data_wsumf[c][count];
                data->m_dec_data_scaled[c][x] = data->m_dec_data_scaled[c][count];
            }
        }
    }

    for (uint32_t c = 0; c < m_pimpl->m_max_channels; c++) {
        if (!m_pimpl->m_channelState[c])
            continue;

        for (int mode = MIN_DSP_MODE; mode < COUNT_DSP_MODE; mode++) {
            max_pw[mode] = std::numeric_limits<cdsp_data_t>::lowest();
            max_pw_idx[mode] = 0;
        }

        for (uint32_t i = 0; i < getOutSignalLength(); i++) {

            auto& converted_z = data->m_dec_data_z[c];
            auto& converted_wsumf = data->m_dec_data_wsumf[c];
            auto& converted_scaled = data->m_dec_data_scaled[c];

            auto& dbm_vec_c = data->m_converted.m_result[DBM][c];
            auto& mw_vec_c = data->m_converted.m_result[MW][c];
            auto& dbw_vec_c = data->m_converted.m_result[DBW][c];
            auto& volt_vec_c = data->m_converted.m_result[VOLT][c];
            auto& dbu_vec_c = data->m_converted.m_result[DBU][c];
            auto& dbv_vec_c = data->m_converted.m_result[DBV][c];
            auto& dbuV_vec_c = data->m_converted.m_result[DBuV][c];

            auto log_scaled_10 = log10f_neon(converted_scaled[i]);
            auto log_converted_10 = log10f_neon(converted_z[i]);
            auto need_skip = isSkip(i);
            /* Conversion to power (Watts) */
            auto ch_p = converted_z[i] * g_w2mw;
            dbm_vec_c[i] = ch_p > LOG_LIMIT ? 10 * (log_converted_10 + LOG_W2MW) : logLim_10;  // W -> mW -> dBm

            if (dbm_vec_c[i] >= max_pw[DBM] && !need_skip) {
                max_pw[DBM] = dbm_vec_c[i];
                max_pw_idx[DBM] = i;
            }

            volt_vec_c[i] = converted_wsumf[i];

            if (volt_vec_c[i] >= max_pw[VOLT] && !need_skip) {
                max_pw[VOLT] = volt_vec_c[i];
                max_pw_idx[VOLT] = i;
            }

            // ( 20*log10( 0.686 / .775 ))
            ch_p = converted_scaled[i] / 0.775;
            // dbu_vec_c[i] = ch_p > LOG_LIMIT ? 20 * log10f_neon(ch_p) : logLim_20;  // W -> mW -> dBm
            dbu_vec_c[i] = ch_p > LOG_LIMIT ? 20 * (log_scaled_10 - LOG_0775) : logLim_20;

            if (dbu_vec_c[i] >= max_pw[DBU] && !need_skip) {
                max_pw[DBU] = dbu_vec_c[i];
                max_pw_idx[DBU] = i;
            }

            // ( 20*log10( RMS / 1.0 ))
            ch_p = converted_scaled[i];
            //dbv_vec_c[i] = ch_p > LOG_LIMIT ? 20 * log10f_neon(ch_p) : logLim_20;  // W -> mW -> dBm
            dbv_vec_c[i] = ch_p > LOG_LIMIT ? 20 * log_scaled_10 : logLim_20;

            if (dbv_vec_c[i] >= max_pw[DBV] && !need_skip) {
                max_pw[DBV] = dbv_vec_c[i];
                max_pw_idx[DBV] = i;
            }

            // ( 20*log10( RMS / 1.0 )) + 120
            // ch_p = converted_scaled[i];
            // dbuV_vec_c[i] = ch_p > LOG_LIMIT ? 20 * log10f_neon(ch_p) + 120 : logLim_20 + 120;  // W -> mW -> dBm
            dbuV_vec_c[i] = dbv_vec_c[i] + 120;
            if (dbuV_vec_c[i] >= max_pw[DBuV] && !need_skip) {
                max_pw[DBuV] = dbuV_vec_c[i];
                max_pw_idx[DBuV] = i;
            }

            // W -> mW
            ch_p = converted_z[i] * g_w2mw;
            mw_vec_c[i] = ch_p;

            if (mw_vec_c[i] >= max_pw[MW] && !need_skip) {
                max_pw[MW] = mw_vec_c[i];
                max_pw_idx[MW] = i;
            }

            ch_p = converted_z[i];
            dbw_vec_c[i] = ch_p > LOG_LIMIT ? 10 * log_converted_10 : logLim_10;

            if (dbw_vec_c[i] >= max_pw[DBW] && !need_skip) {
                max_pw[DBW] = dbw_vec_c[i];
                max_pw_idx[DBW] = i;
            }
        }
        for (int mode = MIN_DSP_MODE; mode < COUNT_DSP_MODE; mode++) {
            data->m_converted.m_peak_power[mode][c] = max_pw[mode];
            data->m_converted.m_peak_freq[mode][c] = ((float)max_pw_idx[mode] / (float)getOutSignalLength() * freq_smpl / 2);
        }
    }

    return 0;
}

auto CDSP::createData() -> data_t* {
    auto max_size = getSignalMaxLength();
    auto max_out_size = getOutSignalMaxLength();
    auto resize = [](uint32_t size, auto& data) {
        data.resize(size);
        if (data.size() != size)
            throw std::exception();
    };

    data_t* d = nullptr;
    try {
        d = new data_t();
        d->m_converted.m_channels = m_pimpl->m_max_channels;
        d->m_in.resize(m_pimpl->m_max_channels);
        d->m_filtred.resize(m_pimpl->m_max_channels);
        d->m_fft.resize(m_pimpl->m_max_channels);
        d->m_dec_data_z.resize(m_pimpl->m_max_channels);
        d->m_dec_data_wsumf.resize(m_pimpl->m_max_channels);
        d->m_dec_data_scaled.resize(m_pimpl->m_max_channels);

        resize(max_out_size, d->m_converted.m_freq_vector);

        for (uint8_t ch = 0; ch < m_pimpl->m_max_channels; ch++) {
            resize(max_size, d->m_in[ch]);
            resize(max_size, d->m_filtred[ch]);
            resize(max_size, d->m_fft[ch]);
            resize(max_out_size, d->m_dec_data_z[ch]);
            resize(max_out_size, d->m_dec_data_wsumf[ch]);
            resize(max_out_size, d->m_dec_data_scaled[ch]);
        }

        for (int mode = MIN_DSP_MODE; mode < COUNT_DSP_MODE; mode++) {
            d->m_converted.m_peak_power[mode].resize(m_pimpl->m_max_channels);
            d->m_converted.m_peak_freq[mode].resize(m_pimpl->m_max_channels);
            d->m_converted.m_result[mode].resize(max_out_size);
            for (uint8_t ch = 0; ch < m_pimpl->m_max_channels; ch++) {
                resize(max_out_size, d->m_converted.m_result[mode][ch]);
            }
        }
        d->m_is_data_filtred = false;
        return d;
    } catch (const std::exception& e) {
        delete d;
        ERROR_LOG("Can not allocate memory");
        return nullptr;
    }
}