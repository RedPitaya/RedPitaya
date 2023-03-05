#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "rp.h"
#include "common.h"

#define SCREEN_BUFF_SIZE 2048

#define TIME_ANIMATION  1000000.0

using namespace std::chrono;

class COscilloscope {
public:

    struct DataPass
    {
        float   ch_min[MAX_ADC_CHANNELS];
        float   ch_max[MAX_ADC_CHANNELS];
        float   ch_avg[MAX_ADC_CHANNELS];
        int32_t ch_min_raw[MAX_ADC_CHANNELS];
        int32_t ch_max_raw[MAX_ADC_CHANNELS];
        int32_t ch_avg_raw[MAX_ADC_CHANNELS];
        uint64_t index;
    };

    struct DataPassSq
    {
        float    wave[SCREEN_BUFF_SIZE];
        int      wave_size;
        rp_channel_t cur_channel;
        uint64_t index;
        DataPassSq(){
            wave_size = 0;
            cur_channel = RP_CH_1;
            index = 0;
        }
    };

    struct DataPassAutoFilter
    {
        double   ampl;
        double   calib_value; // Calib value for AA and BB coff
        double   calib_value_raw;
        double   deviation;
        uint32_t f_aa;
        uint32_t f_bb;
        uint32_t f_pp;
        uint32_t f_kk;
        rp_channel_t cur_channel;
        uint64_t index;
        bool     is_valid;
        DataPassAutoFilter(){
            ampl = 0;
            calib_value = 0;
            calib_value_raw = 0;
            deviation = 0;
            f_aa = f_bb = f_kk = f_pp = 0;
            index = 0;
            is_valid = false;
        }
    };

    struct DataPassAutoFilterSync
    {
        DataPassAutoFilter valueCH[MAX_ADC_CHANNELS];
    };

    using Ptr = std::shared_ptr<COscilloscope>;
    static Ptr Create(uint32_t _decimation);

    COscilloscope(uint32_t _decimation);
    COscilloscope(const COscilloscope &) = delete;
    COscilloscope(COscilloscope &&) = delete;
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
    auto getDataAutoFilterSync() ->DataPassAutoFilterSync;
    auto setZoomMode(bool enable) -> void;
    auto setCursor1(float value) -> void;
    auto setCursor2(float value) -> void;
    auto setHyst(float value) -> void;
    auto setLV() -> void; // 1:1
    auto setHV() -> void; // 1:20
    auto setAcquireChannel(rp_channel_t _ch) -> void;
    auto updateAcqFilter(rp_channel_t _ch) -> void;

    auto setDC() -> void;
    auto setAC() -> void;
    auto setGenGainx1() -> void;
    auto setGenGainx5() -> void;

    auto setGEN_DISABLE() -> void;
    auto setGEN0() -> void;
    auto setGEN0_5() -> void;
    auto setGEN0_5_SINE() -> void;
    auto updateGenCalib() -> void;
    auto enableGen(rp_channel_t _ch,bool _enable) -> void;
    auto resetGen() -> void;
    auto setFreq(rp_channel_t _ch,int _freq) -> int;
    auto setAmp(rp_channel_t _ch,float _ampl) -> int;
    auto setOffset(rp_channel_t _ch,float _offset) -> int;
    auto setGenType(rp_channel_t _ch,int _type) -> int;

private:
    auto startThread() -> void;
    auto oscWorker() -> void;
    auto acquire() -> void;
    auto acquireSquare() -> void;
    auto acquireAutoFilter() -> void;
    auto acquireAutoFilterSync() -> void;
    auto selectRange(float *buffer,double _start,double _stop) -> COscilloscope::DataPassSq;

        std::atomic_flag m_OscThreadRun = ATOMIC_FLAG_INIT;
        std::atomic_bool m_OscThreadRunState;
        std::thread      m_OscThread;
        pthread_mutex_t  m_mutex;
        pthread_mutex_t  m_funcSelector;
        uint32_t         m_decimation;
        uint32_t         m_decimationSq;
        double           m_curCursor1;
        double           m_curCursor2;
        double           m_cursor1;
        double           m_cursor2;
        float            m_hyst;
            microseconds m_startTimeAni;
            microseconds m_lastTimeAni;
        std::atomic_bool m_zoomMode;
        buffers_t        m_buffer;
        // float            m_buffer[MAX_ADC_CHANNELS][ADC_BUFFER_SIZE];
        // uint16_t         m_buffer_raw[ADC_CHANNELS][ADC_BUFFER_SIZE];
        DataPass         m_crossData;
        DataPassSq       m_crossDataSq;
      DataPassAutoFilter m_crossDataAutoFilter;
  DataPassAutoFilterSync m_crossDataAutoFilterSync;
        uint64_t         m_index;
        char             m_mode;
        rp_channel_t     m_channel;
        uint8_t          m_channels;
};
