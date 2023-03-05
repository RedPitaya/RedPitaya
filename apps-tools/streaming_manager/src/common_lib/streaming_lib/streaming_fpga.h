#ifndef STREAMING_LIB_STREAMING_FPGA_H
#define STREAMING_LIB_STREAMING_FPGA_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

#include "uio_lib/oscilloscope.h"
#include "data_lib/signal.hpp"
#include "data_lib/buffer.h"
#include "data_lib/buffers_pack.h"
#include "data_lib/thread_cout.h"

namespace streaming_lib {

class CStreamingFPGA
{

public:
    using Ptr = std::shared_ptr<CStreamingFPGA>;

    CStreamingFPGA(uio_lib::COscilloscope::Ptr _osc, uint8_t _adc_bits);
    ~CStreamingFPGA();

    typedef std::function<DataLib::CDataBuffersPack::Ptr(uint64_t)> getFreeBufferFunc;
    typedef std::function<void()> unlockBufferFunc;


    auto addChannel(DataLib::EDataBuffersPackChannel _channel,DataLib::CDataBuffer::ADC_MODE _adc_mode, uint8_t _bitsBySample) -> void;
    auto runNonBlock() -> void;

    auto stop() -> bool;
    auto isRun() -> bool;
    auto setTestMode(bool mode) -> void;
    auto setVerbousMode(bool mode) -> void;
    auto setPrintDebugBuffer(bool mode) -> void;

    sigslot::signal<DataLib::CDataBuffersPack::Ptr> oscNotify;
    sigslot::signal<bool> isRunNotify;

    getFreeBufferFunc getBuffF;
    unlockBufferFunc  unlockBuffF;

private:

    CStreamingFPGA(const CStreamingFPGA &) = delete;
    CStreamingFPGA(CStreamingFPGA &&) = delete;
    CStreamingFPGA& operator=(const CStreamingFPGA&) =delete;
    CStreamingFPGA& operator=(const CStreamingFPGA&&) =delete;


    struct SADCsettings{
        uint8_t m_bits;
        DataLib::CDataBuffer::ADC_MODE m_mode;
    };

    uio_lib::COscilloscope::Ptr m_Osc_ch;

    std::thread m_OscThread;
    std::mutex mtx;
    std::atomic_bool m_OscThreadRun;
    std::atomic_bool m_isRun;

    uint64_t         m_passRate;
    uint8_t          m_adc_bits;

    uintmax_t        m_BytesCount;
    bool             m_testMode;
    bool             m_verbMode;
    bool             m_printDebugBuffer;

    std::map<DataLib::EDataBuffersPackChannel,SADCsettings> m_adcSettings;

    auto oscWorker() -> void;
    auto passCh() -> DataLib::CDataBuffersPack::Ptr;
    auto prepareTestBuffers() -> void;
    auto setIsRun(bool state) -> void;

    uint8_t *m_testBuffer;
};

}

#endif
