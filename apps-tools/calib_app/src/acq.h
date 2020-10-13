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
            float ch1_min;
            float ch1_max;
            float ch1_avg;
            float ch2_min;
            float ch2_max;
            float ch2_avg;
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
    private:
        void oscWorker();
        void acquire();
        std::atomic_flag m_OscThreadRun = ATOMIC_FLAG_INIT;
        std::thread      m_OscThread;
        pthread_mutex_t  m_mutex;
        uint32_t         m_decimation;
        float            m_buffer[2][ADC_BUFFER_SIZE];
        DataPass         m_crossData;
};
