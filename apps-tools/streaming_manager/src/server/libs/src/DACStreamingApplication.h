#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <asio.hpp>

#include <Generator.h>
#include <DACStreamingManager.h>


class CDACStreamingApplication
{

public:
    CDACStreamingApplication(CDACStreamingManager::Ptr _streamingManager, CGenerator::Ptr _gen);
    ~CDACStreamingApplication();
    auto run() -> void;
    auto runNonBlock() -> void;
    auto stop(bool wait = true) -> bool;
    auto isRun() -> bool {return m_isRun;}
    auto setTestMode(bool mode) -> void;
    auto setVerbousMode(bool mode) -> void;

private:
    int m_PerformanceCounterPeriod = 10;

    CGenerator::Ptr m_gen;
    CDACStreamingManager::Ptr m_streamingManager;
    std::thread m_Thread;
    std::mutex mtx;
    std::atomic_flag m_GenThreadRun = ATOMIC_FLAG_INIT;
    std::atomic_int  m_ReadyToPass;
    std::atomic_bool m_isRun;
    std::atomic_bool m_isRunNonBloking;
    static_assert(ATOMIC_INT_LOCK_FREE == 2,"this implementation does not guarantee that std::atomic<int> is always lock free.");
    bool             m_testMode;
    bool             m_verbMode;
    
    void genWorker();
    void signalHandler(const asio::error_code &_error, int _signalNumber);
};
