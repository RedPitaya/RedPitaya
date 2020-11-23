#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "rp.h"
#define SCREEN_BUFF_SIZE 2048

#define TIME_ANIMATION  1000000.0

using namespace std::chrono;

class COscilloscope {
    public:

        struct DataPass
        {
            float   ch1_min;
            float   ch1_max;
            float   ch1_avg;
            float   ch2_min;
            float   ch2_max;
            float   ch2_avg;
            int32_t ch1_min_raw;
            int32_t ch1_max_raw;
            int32_t ch1_avg_raw;
            int32_t ch2_min_raw;
            int32_t ch2_max_raw;
            int32_t ch2_avg_raw;
            uint64_t index; 
        };

         struct DataPassSq
        {
            float    wave[SCREEN_BUFF_SIZE];
            int      wave_size;
            rp_channel_t cur_channel;
            uint64_t index; 
        };

        using Ptr = std::shared_ptr<COscilloscope>;
        static Ptr Create(uint32_t _decimation);

        COscilloscope(uint32_t _decimation);
        COscilloscope(const COscilloscope &) = delete;
        COscilloscope(COscilloscope &&) = delete;
        ~COscilloscope();

        void start();
        void startNormal();
        void startSquare(uint32_t _decimation);
        void stop();
    DataPass getData();
  DataPassSq getDataSq();
        void setZoomMode(bool enable);
        void setCursor1(float value);
        void setCursor2(float value);
        void setLV(); // 1:1
        void setHV(); // 1:20
        void setAcquireChannel(rp_channel_t _ch);
#ifdef Z20_250_12
        void setDC();
        void setAC();
        void setGenGainx1();
        void setGenGainx5();
#endif
        void setGEN_DISABLE();
        void setGEN0();
        void setGEN0_5();
        void setGEN0_5_SINE();
        void updateGenCalib();
        void enableGen(rp_channel_t _ch,bool _enable);
        void resetGen();
         int setFreq(rp_channel_t _ch,int _freq);
         int setAmp(rp_channel_t _ch,float _ampl);
         int setOffset(rp_channel_t _ch,float _offset);
         int setGenType(rp_channel_t _ch,int _type);
         
    private:
                        void startThread();
                        void oscWorker();
                        void acquire();
                        void acquireSquare();
   COscilloscope::DataPassSq selectRange(float *buffer,double _start,double _stop);
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
                microseconds m_startTimeAni;
                microseconds m_lastTimeAni;
            std::atomic_bool m_zoomMode;
            float            m_buffer[2][ADC_BUFFER_SIZE];
            uint16_t         m_buffer_raw[2][ADC_BUFFER_SIZE];
            DataPass         m_crossData;
            DataPassSq       m_crossDataSq;
            uint64_t         m_index;
            char             m_mode;
            rp_channel_t     m_channel;
};
