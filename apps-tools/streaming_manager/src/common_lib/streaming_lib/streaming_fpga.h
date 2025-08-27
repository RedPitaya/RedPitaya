#ifndef STREAMING_LIB_STREAMING_FPGA_H
#define STREAMING_LIB_STREAMING_FPGA_H

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

#include "data_lib/buffers_pack.h"
#include "data_lib/signal.hpp"
#include "uio_lib/oscilloscope.h"

namespace streaming_lib {

class CStreamingFPGA {
   public:
    using Ptr = std::shared_ptr<CStreamingFPGA>;

    CStreamingFPGA(uio_lib::COscilloscope::Ptr _osc, uint8_t _adc_bits);
    ~CStreamingFPGA();

    typedef std::function<DataLib::CDataBuffersPackDMA::Ptr()> getFreeBufferFunc;
    typedef std::function<void()> unlockBufferFunc;

    auto runNonBlock() -> void;

    auto stop() -> bool;
    auto isRun() -> bool;
    auto setTestMode(bool mode) -> void;
    auto setVerbousMode(bool mode) -> void;
    auto setPrintDebugBuffer(bool mode) -> void;

    sigslot::signal<DataLib::CDataBuffersPackDMA::Ptr> oscNotify;
    sigslot::signal<bool> isRunNotify;

    getFreeBufferFunc getBuffF;
    unlockBufferFunc unlockBuffF;

   private:
    CStreamingFPGA(const CStreamingFPGA&) = delete;
    CStreamingFPGA(CStreamingFPGA&&) = delete;
    CStreamingFPGA& operator=(const CStreamingFPGA&) = delete;
    CStreamingFPGA& operator=(const CStreamingFPGA&&) = delete;

    uio_lib::COscilloscope::Ptr m_Osc_ch;

    std::thread m_OscThread;
    std::mutex mtx;
    std::atomic_bool m_OscThreadRun;
    std::atomic_bool m_isRun;

    uint64_t m_passRate;
    uint8_t m_adc_bits;

    uintmax_t m_BytesCount;
    bool m_testMode;
    bool m_verbMode;
    bool m_printDebugBuffer;

    uint64_t m_overFlowSumm;
    uint64_t m_overFlowSummCount;

    DataLib::CDataBuffersPackDMA::Ptr m_mappedBuffers[2];
    uint8_t m_currentBuffer;

    auto oscWorker() -> void;
    auto passCh() -> DataLib::CDataBuffersPackDMA::Ptr;
    auto prepareTestBuffers() -> void;
    auto setIsRun(bool state) -> void;
};

}  // namespace streaming_lib

#endif
