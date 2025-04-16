#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "rp.h"

#define RP_CALIB_SCREEN_BUFF_SIZE 2048
#define RP_CALIB_MAX_ADC_CHANNELS 4
#define RP_CALIB_MAX_DAC_CHANNELS 2

namespace rp_calib {

class COscilloscope {
   public:
    struct DataPass {
        float ch_min[RP_CALIB_MAX_ADC_CHANNELS];
        float ch_max[RP_CALIB_MAX_ADC_CHANNELS];
        float ch_avg[RP_CALIB_MAX_ADC_CHANNELS];
        float ch_p_p[RP_CALIB_MAX_ADC_CHANNELS];
        int32_t ch_min_raw[RP_CALIB_MAX_ADC_CHANNELS];
        int32_t ch_max_raw[RP_CALIB_MAX_ADC_CHANNELS];
        int32_t ch_avg_raw[RP_CALIB_MAX_ADC_CHANNELS];
        int32_t periodsByBuffer[RP_CALIB_MAX_ADC_CHANNELS];
        bool isSineSignal[RP_CALIB_MAX_ADC_CHANNELS];
        uint64_t index;
        DataPass() {
            for (auto i = 0u; i < RP_CALIB_MAX_ADC_CHANNELS; i++) {
                ch_min[i] = ch_max[i] = ch_avg[i] = ch_p_p[i] = ch_min_raw[i] = ch_max_raw[i] = ch_avg_raw[i] = periodsByBuffer[i] = 0;
                isSineSignal[i] = false;
            }
            index = 0;
        }
    };

    struct DataPassSq {
        float wave[RP_CALIB_SCREEN_BUFF_SIZE];
        int wave_size;
        rp_channel_t cur_channel;
        uint64_t index;
        DataPassSq() {
            wave_size = 0;
            cur_channel = RP_CH_1;
            index = 0;
        }
    };

    struct DataPassAutoFilter {
        double ampl;
        double calib_value;  // Calib value for AA and BB coff
        double calib_value_raw;
        double rmsFilter;
        double deviation;
        uint32_t f_aa;
        uint32_t f_bb;
        uint32_t f_pp;
        uint32_t f_kk;
        rp_channel_t cur_channel;
        uint64_t index;
        bool is_valid;
        DataPassAutoFilter() {
            ampl = 0;
            rmsFilter = 0;
            calib_value = 0;
            calib_value_raw = 0;
            deviation = 0;
            f_aa = f_bb = f_kk = f_pp = 0;
            index = 0;
            is_valid = false;
        }
    };

    struct DataPassAutoFilterSync {
        DataPassAutoFilter valueCH[RP_CALIB_MAX_ADC_CHANNELS];
    };

    using Ptr = std::shared_ptr<COscilloscope>;
    static Ptr Create(uint32_t _decimation);

    COscilloscope(uint32_t _decimation);
    COscilloscope(const COscilloscope&) = delete;
    COscilloscope(COscilloscope&&) = delete;
    ~COscilloscope();

    auto start() -> void;
    auto startNormal() -> void;
    auto startSquare(uint32_t _decimation) -> void;
    auto startAutoFilter(uint32_t _decimation) -> void;
    auto startAutoFilterNCh(uint32_t _decimation) -> void;
    auto cancel() -> void;
    auto stop() -> void;
    auto getData() -> DataPass;
    auto getDataSq() -> DataPassSq;
    auto getDataAutoFilter() -> DataPassAutoFilter;
    auto getDataAutoFilterSync() -> DataPassAutoFilterSync;
    auto setZoomMode(bool enable) -> void;
    auto setCursor1(float value) -> void;
    auto setCursor2(float value) -> void;
    auto setHyst(float value) -> void;
    auto setLV() -> void;  // 1:1
    auto setHV() -> void;  // 1:20
    auto setAcquireChannel(rp_channel_t _ch) -> void;
    auto updateAcqFilter(rp_channel_t _ch) -> void;
    auto setDeciamtion(uint32_t _decimation) -> void;
    auto getDecimation() -> uint32_t;

    auto setDC() -> void;
    auto setAC() -> void;
    auto setGenGainx1() -> void;
    auto setGenGainx5() -> void;

    auto setGEN_DISABLE() -> void;
    auto setGEN0() -> void;
    auto setGEN0_5() -> void;
    auto setGEN0_5_SINE() -> void;
    auto updateGenCalib() -> void;
    auto enableGen(rp_channel_t _ch, bool _enable) -> void;
    auto resetGen() -> void;
    auto setFreq(rp_channel_t _ch, int _freq) -> int;
    auto setAmp(rp_channel_t _ch, float _ampl) -> int;
    auto setOffset(rp_channel_t _ch, float _offset) -> int;
    auto setGenType(rp_channel_t _ch, int _type) -> int;

    auto setAvgFilter(bool _enable) -> void;
    auto getAvgFilter() -> bool;
    auto resetAvgFilter() -> void;

   private:
    auto startThread() -> void;
    auto oscWorker() -> void;
    auto acquire() -> void;
    auto acquireSquare() -> void;
    auto acquireAutoFilter() -> void;
    auto acquireAutoFilterSync() -> void;
    auto selectRange(float* buffer, double _start, double _stop) -> COscilloscope::DataPassSq;
    auto measurePeriod(int16_t* _data, uint32_t _size, double* period, uint32_t _decimation) -> void;

    std::atomic_flag m_OscThreadRun = ATOMIC_FLAG_INIT;
    std::atomic_bool m_OscThreadRunState;
    std::thread m_OscThread;
    pthread_mutex_t m_mutex;
    pthread_mutex_t m_funcSelector;
    pthread_mutex_t m_avgFilter;
    uint32_t m_decimation;
    uint32_t m_decimationSq;
    double m_curCursor1;
    double m_curCursor2;
    double m_cursor1;
    double m_cursor2;
    float m_hyst;
    std::chrono::microseconds m_startTimeAni;
    std::chrono::microseconds m_lastTimeAni;
    std::atomic_bool m_zoomMode;
    buffers_t m_buffer;
    // float            m_buffer[MAX_ADC_CHANNELS][ADC_BUFFER_SIZE];
    // uint16_t         m_buffer_raw[ADC_CHANNELS][ADC_BUFFER_SIZE];
    DataPass m_crossData;
    DataPassSq m_crossDataSq;
    DataPassAutoFilter m_crossDataAutoFilter;
    DataPassAutoFilterSync m_crossDataAutoFilterSync;
    uint64_t m_index;
    char m_mode;
    rp_channel_t m_channel;
    uint8_t m_channels;
    bool m_avg_filter;
    uint32_t m_avg_filter_size;
    uint32_t m_avg_filter_cur;
    float* m_avg_filter_buffer_p_p[RP_CALIB_MAX_ADC_CHANNELS];
    float* m_avg_filter_buffer_min[RP_CALIB_MAX_ADC_CHANNELS];
    float* m_avg_filter_buffer_max[RP_CALIB_MAX_ADC_CHANNELS];
    float* m_avg_filter_buffer_mean[RP_CALIB_MAX_ADC_CHANNELS];
    double m_adc_sample_per;
};

}  // namespace rp_calib