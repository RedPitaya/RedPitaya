#ifndef STREAMING_LIB_STREAMING_FPGA_H
#define STREAMING_LIB_STREAMING_FPGA_H

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

#include "data_lib/buffers_pack.h"
#include "data_lib/signal.hpp"
#include "uio_lib/hardware.h"

namespace streaming_lib {

class CStreamingFPGA {
   public:
    using Ptr = std::shared_ptr<CStreamingFPGA>;

    CStreamingFPGA(uio_lib::CHardware::Ptr _hw, bool _saveCaptureTime);
    ~CStreamingFPGA();

    typedef std::function<DataLib::CDataBuffersPackDMA::Ptr()> getFreeBufferFunc;
    typedef std::function<void()> unlockBufferFunc;

    auto runNonBlock() -> void;

    auto stop() -> bool;
    auto isRun() -> bool;
    auto setTestMode(bool mode) -> void;
    auto setVerboseMode(bool mode) -> void;
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

    uio_lib::CHardware::Ptr m_hw;

    std::thread m_OscThread;
    std::mutex mtx;
    std::atomic_bool m_OscThreadRun;
    std::atomic_bool m_isRun;

    uint64_t m_passRate;
    bool m_saveCaptureTime;

    uintmax_t m_BytesCount;
    bool m_testMode;
    bool m_verbMode;
    bool m_printDebugBuffer;

    uint64_t m_overFlowSumm[4];  // per channel, of the buffers dropped for want of a free pack
    uint64_t m_overFlowSummCount;
    uint64_t m_waitSumm;  // the trigger wait before the dropped buffers

    // the same for every buffer of a run
    uint32_t m_decimation;
    uint32_t m_adcClock;
    uint16_t m_triggerBlocks;
    uint8_t m_adcFlags;

    DataLib::CDataBuffersPackDMA::Ptr m_mappedBuffers[2];
    uint8_t m_currentBuffer;

    auto oscWorker() -> void;
    auto passCh() -> DataLib::CDataBuffersPackDMA::Ptr;
    auto prepareTestBuffers() -> void;
    auto setIsRun(bool state) -> void;
};

}  // namespace streaming_lib

#endif
