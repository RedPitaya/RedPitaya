#ifndef STREAMING_ROOT_DACSAPP_H
#define STREAMING_ROOT_DACSAPP_H

#include <atomic>
#include <mutex>
#include <thread>

#include "dac_streaming_manager.h"
#include "uio_lib/hardware.h"

namespace dac_streaming_lib {

class CDACStreamingApplication {
   public:
    using Ptr = std::shared_ptr<CDACStreamingApplication>;

    CDACStreamingApplication(CDACStreamingManager::Ptr _streamingManager, uio_lib::CHardware::Ptr _hw);
    ~CDACStreamingApplication();
    auto run() -> void;
    // managerRunning: the caller started the manager itself (the data port is open)
    auto runNonBlock(bool managerRunning = false) -> void;
    auto stop() -> bool;
    auto isRun() -> bool { return m_isRun; }
    auto setVerboseMode(bool mode) -> void;

   private:
    void genWorker();
    void signalHandler(const std::error_code& _error, int _signalNumber);

    int m_PerformanceCounterPeriod = 10;
    uio_lib::CHardware::Ptr m_hw;
    CDACStreamingManager::Ptr m_streamingManager;
    std::thread m_Thread;
    std::mutex mtx;
    std::atomic_bool m_GenThreadRun;
    std::atomic_int m_ReadyToPass;
    std::atomic_bool m_isRun;
    std::atomic_bool m_isRunNonBloking;
    bool m_verbMode;
};

}  // namespace dac_streaming_lib
#endif
