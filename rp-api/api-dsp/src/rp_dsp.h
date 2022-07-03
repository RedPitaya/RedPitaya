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

    typedef struct{
        double **in = nullptr;
        double **filtred = nullptr;
        bool is_data_filtred = false;
        double **fft = nullptr;
        float  *freq_vector = nullptr;
        float  **decimated = nullptr;
        float  **converted = nullptr;
        float  *peak_power = nullptr;
        float  *peak_freq = nullptr;
        uint8_t channels = 0;
        auto reset() -> void{
            is_data_filtred = false;
        }
    } data_t;

    CDSP(uint8_t max_channels,uint32_t adc_buffer,uint32_t adc_max_speed);
    ~CDSP();

    auto createData() -> data_t *;
    auto deleteData(data_t *data) -> void;

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

    
    auto prepareFreqVector(data_t *data, double f_s, float decimation) -> int;

    auto windowFilter(data_t *data) -> int;
    auto fftInit() -> int;
    auto fftClean() -> int;
    auto fft(data_t *data) -> int;
    auto decimate(data_t *data,uint32_t in_len, uint32_t out_len) -> int;
    auto cnvToDBM(data_t *data,uint32_t  decimation) -> int;
    auto cnvToDBMMaxValueRanged(data_t *data,uint32_t  decimation,uint32_t minFreq,uint32_t maxFreq) -> int;
    auto cnvToMetric(data_t *data,uint32_t  decimation) -> int;

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
