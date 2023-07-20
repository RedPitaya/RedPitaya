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

    typedef struct{
        double **m_in = nullptr;
        double **m_filtred = nullptr;
        bool m_is_data_filtred = false;
        double **m_fft = nullptr;
        float  *m_freq_vector = nullptr;
        float  **m_decimated = nullptr;
        float  **m_converted = nullptr;
        float  *m_peak_power = nullptr;
        float  *m_peak_freq = nullptr;
        uint8_t m_channels = 0;
        auto reset() -> void{
            m_is_data_filtred = false;
        }
    } data_t;


class CDSP{

public:


    CDSP(uint8_t max_channels,uint32_t adc_buffer,uint32_t adc_max_speed);
    ~CDSP();

    data_t * createData();
    void deleteData(data_t *data);

    void setChannel(uint8_t ch, bool enable);
    int setSignalLength(uint32_t len);
    uint32_t getSignalLength();
    uint32_t getSignalMaxLength();
    uint32_t getOutSignalLength();
    uint32_t getOutSignalMaxLength();

    int window_init(window_mode_t mode);
    int window_clean();
    window_mode_t getCurrentWindowMode();

    void setImpedance(double value);
    double getImpedance();

    void setRemoveDC(bool enable);
    bool getRemoveDC();

    void setMode(mode_t mode);
    mode_t getMode();

    
    int prepareFreqVector(data_t *data, double adc_rate_f_s, float decimation);

    int windowFilter(data_t *data);
    int fftInit();
    int fftClean();
    int fft(data_t *data);
    int decimate(data_t *data,uint32_t in_len, uint32_t out_len);
    int cnvToDBM(data_t *data,uint32_t  decimation);
    int cnvToDBMMaxValueRanged(data_t *data,uint32_t  decimation,uint32_t minFreq,uint32_t maxFreq);
    int cnvToMetric(data_t *data,uint32_t  decimation);
    uint8_t remoteDCCount();

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

#endif
