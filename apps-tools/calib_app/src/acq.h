#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "rp.h"

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

        using Ptr = std::shared_ptr<COscilloscope>;
        static Ptr Create(uint32_t _decimation);

        COscilloscope(uint32_t _decimation);
        COscilloscope(const COscilloscope &) = delete;
        COscilloscope(COscilloscope &&) = delete;
        ~COscilloscope();

        void start();
        void stop();
    DataPass getData();
        void setLV(); // 1:1
        void setHV(); // 1:20
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

    private:
        void oscWorker();
        void acquire();
        std::atomic_flag m_OscThreadRun = ATOMIC_FLAG_INIT;
        std::thread      m_OscThread;
        pthread_mutex_t  m_mutex;
        uint32_t         m_decimation;
        float            m_buffer[2][ADC_BUFFER_SIZE];
        uint16_t         m_buffer_raw[2][ADC_BUFFER_SIZE];
        DataPass         m_crossData;
        uint64_t         m_index; 
};
