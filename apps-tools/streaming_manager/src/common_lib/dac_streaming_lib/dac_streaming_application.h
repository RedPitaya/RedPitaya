#ifndef STREAMING_ROOT_DACSAPP_H
#define STREAMING_ROOT_DACSAPP_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "uio_lib/generator.h"
#include "dac_streaming_manager.h"

namespace dac_streaming_lib {

class CDACStreamingApplication
{

public:

    using Ptr = std::shared_ptr<CDACStreamingApplication>;

    CDACStreamingApplication(CDACStreamingManager::Ptr _streamingManager, uio_lib::CGenerator::Ptr _gen);
    ~CDACStreamingApplication();
    auto run() -> void;
    auto runNonBlock() -> void;
    auto stop(bool wait) -> bool;
    auto isRun() -> bool {return m_isRun;}

private:
    int m_PerformanceCounterPeriod = 10;

     uio_lib::CGenerator::Ptr m_gen;
    CDACStreamingManager::Ptr m_streamingManager;
    std::thread m_Thread;
    std::mutex mtx;
    std::atomic_flag m_GenThreadRun = ATOMIC_FLAG_INIT;
    std::atomic_int  m_ReadyToPass;
    std::atomic_bool m_isRun;
    std::atomic_bool m_isRunNonBloking;
    static_assert(ATOMIC_INT_LOCK_FREE == 2,"this implementation does not guarantee that std::atomic<int> is always lock free.");

    void genWorker();
    void signalHandler(const std::error_code &_error, int _signalNumber);
};

}
#endif
