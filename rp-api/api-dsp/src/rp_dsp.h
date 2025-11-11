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
#include <array>
#include <vector>

namespace rp_dsp_api {

typedef enum { RECTANGULAR = 0, HANNING = 1, HAMMING = 2, BLACKMAN_HARRIS = 3, FLAT_TOP = 4, KAISER_4 = 5, KAISER_8 = 6 } window_mode_t;

typedef enum { DBM = 0, VOLT = 1, DBU = 2, DBV = 3, DBuV = 4, MW = 5, DBW = 6 } mode_t;

#define MIN_DSP_MODE rp_dsp_api::DBM
#define COUNT_DSP_MODE (rp_dsp_api::DBW + 1)

typedef float cdsp_data_t;
typedef std::vector<cdsp_data_t> cdsp_data_vec_t;
typedef std::vector<std::vector<cdsp_data_t>> cdsp_data_ch_t;

typedef struct rp_dsp_result {
    cdsp_data_vec_t m_freq_vector;
    std::array<cdsp_data_ch_t, COUNT_DSP_MODE> m_result;
    std::array<cdsp_data_vec_t, COUNT_DSP_MODE> m_peak_power;
    std::array<cdsp_data_vec_t, COUNT_DSP_MODE> m_peak_freq;
    uint32_t m_maxFreq = 0;
    uint8_t m_channels = 0;
    size_t m_data_size = 0;
} rp_dsp_result_t;

typedef struct rp_dsp_dec_data {
    cdsp_data_ch_t m_decimated;
} rp_dsp_dec_data_t;

typedef struct {
    cdsp_data_ch_t m_in;
    cdsp_data_ch_t m_filtred;
    cdsp_data_ch_t m_fft;
    cdsp_data_ch_t m_dec_data_z;
    cdsp_data_ch_t m_dec_data_wsumf;
    cdsp_data_ch_t m_dec_data_scaled;
    rp_dsp_result_t m_converted;
    bool m_is_data_filtred = false;
    auto reset() -> void { m_is_data_filtred = false; }
} data_t;

class CDSP {

   public:
    CDSP(uint8_t max_channels, uint32_t adc_buffer, uint32_t adc_max_speed, bool createStoredData = false);
    ~CDSP();

    data_t* createData();
    data_t* getStoredData();

    void setChannel(uint8_t ch, bool enable);
    void getChannel(uint8_t ch, bool* enable);
    int setSignalLength(uint32_t len);
    int setSignalLengthDiv2(uint32_t len);
    uint32_t getSignalLength();
    uint32_t getSignalMaxLength();
    uint32_t getOutSignalLength();
    uint32_t getOutSignalMaxLength();

    int window_init(window_mode_t mode);
    window_mode_t getCurrentWindowMode();

    void setImpedance(double value);
    double getImpedance();

    void setRemoveDC(bool enable);
    bool getRemoveDC();

    void setProbe(uint8_t channel, uint32_t value);
    void getProbe(uint8_t channel, uint32_t* value);

    int prepareFreqVector(data_t* data, double adc_rate_f_s, float decimation);
    int prepareFreqVector(data_t* data, float decimation);

    int windowFilter(data_t* data);
    int fftInit();
    int fft(data_t* data);
    int getAmpAndPhase(data_t* _data, double _freq, double* _amp1, double* _phase1, double* _amp2, double* _phase2);

    int decimate(data_t* data, uint32_t in_len, uint32_t out_len);
    // int cnvToDBM(data_t* data, uint32_t decimation);
    // int cnvToDBMMaxValueRanged(data_t* data, uint32_t decimation, uint32_t minFreq = 0, uint32_t maxFreq = 0);
    int cnvToMetric(data_t* data, uint32_t decimation, uint32_t minFreq = 0, uint32_t maxFreq = 0);
    uint8_t remoteDCCount();

   private:
    CDSP(const CDSP&) = delete;
    CDSP(CDSP&&) = delete;
    CDSP& operator=(const CDSP&) = delete;
    CDSP& operator=(const CDSP&&) = delete;

    int decimate_ex(data_t* data, uint32_t in_len, uint32_t out_len);
    int decimate_disabled(data_t* data, uint32_t len);

    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

}  // namespace rp_dsp_api

#endif
