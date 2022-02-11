#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <asio.hpp>

#include <Oscilloscope.h>
#include <StreamingManager.h>

//#define DISABLE_OSC

class CStreamingApplication
{

public:
    CStreamingApplication(CStreamingManager::Ptr _StreamingManager, COscilloscope::Ptr _osc_ch,unsigned short _resolution,int _oscRate, int _channels, int _adc_mode , uint32_t _adc_bits);
    ~CStreamingApplication();
    auto run(std::string _file_name_prefix) -> void;
    auto runNonBlock(std::string _file_name_prefix) -> void;
    auto runNonBlockNoADC(std::string _file_name_prefix) -> void;
    auto runADC() -> void;

    auto stop(bool wait = true) -> bool;
    auto isRun() -> bool {return m_isRun;}
    auto setTestMode(bool mode) -> void;
    auto setVerbousMode(bool mode) -> void;
    auto setPrintDebugBuffer(bool mode) -> void {m_printDebugBuffer = mode;};

private:
    int m_PerformanceCounterPeriod = 10;

    COscilloscope::Ptr m_Osc_ch;
    CStreamingManager::Ptr m_StreamingManager;
    std::thread m_OscThread;
    std::mutex mtx;
    std::atomic_flag m_OscThreadRun = ATOMIC_FLAG_INIT;
    std::atomic_int  m_ReadyToPass;
    std::atomic_bool m_isRun;
    std::atomic_bool m_isRunNonBloking;
    std::atomic_bool m_isRunADC;
    static_assert(ATOMIC_INT_LOCK_FREE == 2,"this implementation does not guarantee that std::atomic<int> is always lock free.");

    unsigned short m_Resolution;

    void *m_WriteBuffer_ch1;
    void *m_WriteBuffer_ch2;
    
    void *m_testBuffer_ch1;
    void *m_testBuffer_ch2;
    
    size_t m_size_ch1;
    size_t m_size_ch2;
    uint64_t         m_lostRate;
    int              m_oscRate;
    int              m_channels;
    int              m_adc_mode;
    uint32_t         m_adc_bits;   
  
    uintmax_t        m_BytesCount;
    bool             m_testMode;
    bool             m_verbMode;
    bool             m_printDebugBuffer;

    auto oscWorker() -> void;
    auto passCh(int _bufferIndex, size_t &_size1,size_t &_size2) -> uint32_t;
    auto oscNotify(uint64_t _lostRate, uint32_t _oscRate, uint32_t _adc_mode, uint32_t _adc_bits,const void *_buffer_ch1, size_t _size_ch1,const void *_buffer_ch2, size_t _size_ch2) -> int;
    auto signalHandler(const asio::error_code &_error, int _signalNumber) -> void;
    auto prepareTestBuffers() -> void;
};
