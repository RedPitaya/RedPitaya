#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>

#include "logger_lib/file_logger.h"
#include "logger_lib/profiler.h"
#include "data_lib/network_header.h"
#include "streaming_fpga.h"

using namespace streaming_lib;

CStreamingFPGA::CStreamingFPGA(uio_lib::COscilloscope::Ptr _osc, bool _saveCaptureTime)
    : m_Osc_ch(_osc),
      m_OscThread(),
      mtx(),
      m_isRun(false),
      m_saveCaptureTime(_saveCaptureTime),
      m_BytesCount(0),
      m_testMode(false),
      m_verbMode(false),
      m_printDebugBuffer(false),
      m_overFlowSumm{0, 0, 0, 0},
      m_overFlowSummCount(0),
      m_waitSumm(0),
      m_decimation(0),
      m_adcClock(0),
      m_triggerBlocks(0),
      m_adcFlags(0),
      m_mappedBuffers{nullptr, nullptr} {
    m_passRate = 0;
    m_OscThreadRun = false;
    getBuffF = nullptr;
    unlockBuffF = nullptr;
    if (!m_Osc_ch) {
        FATAL("Register controller is not initialized")
    }
}

CStreamingFPGA::~CStreamingFPGA() {
    stop();
    TRACE("Exit")
}

auto CStreamingFPGA::isRun() -> bool {
    return m_isRun;
}

auto CStreamingFPGA::setPrintDebugBuffer(bool mode) -> void {
    m_printDebugBuffer = mode;
}

auto CStreamingFPGA::runNonBlock() -> void {
    std::lock_guard lock(mtx);
    try {
        m_OscThreadRun = true;
        setIsRun(true);
        m_OscThread = std::thread(&CStreamingFPGA::oscWorker, this);

#ifdef RP_PLATFORM
        pthread_attr_t thAttr;
        int policy = 0;
        int max_prio_for_policy = 0;
        pthread_attr_init(&thAttr);
        pthread_attr_getschedpolicy(&thAttr, &policy);
        max_prio_for_policy = sched_get_priority_max(policy);
        pthread_setschedprio(m_OscThread.native_handle(), max_prio_for_policy);
        pthread_attr_destroy(&thAttr);
#endif

    } catch (const std::system_error& e) {
        FATAL("%s", e.what())
    }
}

auto CStreamingFPGA::stop() -> bool {
    std::lock_guard lock(mtx);
    m_OscThreadRun = false;
    if (m_OscThread.joinable()) {
        m_OscThread.join();
    }
    m_Osc_ch->stop();
    m_mappedBuffers[0] = m_mappedBuffers[1] = nullptr;
    TRACE("Stop")
    return true;
}

auto CStreamingFPGA::setIsRun(bool state) -> void {
    if (m_isRun != state) {
        m_isRun = state;
        isRunNotify(m_isRun);
    }
}

void CStreamingFPGA::oscWorker() {
    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
    auto value = curTime.time_since_epoch();
    long long int timeBegin = value.count();

    m_passRate = 0;
    m_currentBuffer = 0;

    auto pack = getBuffF();
    if (pack == nullptr) {
        FATAL("Can't get first buffer")
    }
    auto pack2 = getBuffF();
    if (pack2 == nullptr) {
        FATAL("Can't get second buffer")
    }

    if (!m_testMode) {
        m_Osc_ch->setDataAddress(0,
                                 pack->getBufferDataAddress(DataLib::CH1),
                                 pack->getBufferDataAddress(DataLib::CH2),
                                 pack->getBufferDataAddress(DataLib::CH3),
                                 pack->getBufferDataAddress(DataLib::CH4));
        m_Osc_ch->setDataAddress(1,
                                 pack2->getBufferDataAddress(DataLib::CH1),
                                 pack2->getBufferDataAddress(DataLib::CH2),
                                 pack2->getBufferDataAddress(DataLib::CH3),
                                 pack2->getBufferDataAddress(DataLib::CH4));
    }
    m_mappedBuffers[0] = pack;
    m_mappedBuffers[1] = pack2;
    m_decimation = m_Osc_ch->getDecimation();
    m_adcClock = m_Osc_ch->getADCClock();
    m_triggerBlocks = m_Osc_ch->getTriggerBlocks();
    m_adcFlags = (m_Osc_ch->getTriggerMode() ? DataLib::ADC_FLAG_TRIGGER_MODE : 0) | (m_Osc_ch->hasWaitCounter() ? DataLib::ADC_FLAG_WAIT_COUNTER : 0);
    m_Osc_ch->prepare();

    try {
        uint64_t dataSize = 0;
        uint64_t lostSize = 0;
        uint64_t totalPassRate = 0;
        for (auto& s : m_overFlowSumm)
            s = 0;
        m_overFlowSummCount = 0;
        m_waitSumm = 0;
        while (m_OscThreadRun) {
            bool state = true;
            profiler::setTimePoint("FPGA");
            state = m_Osc_ch->wait();
            profiler::printuS("FPGA", "Time write buffer %d", m_currentBuffer);
            if (state) {
                pack = this->passCh();
                m_passRate++;
                totalPassRate++;
            }
            if (state) {
#ifndef RP_PLATFORM
                usleep(10000);  // 10ms
#endif
                oscNotify(pack);
                if (pack) {
                    dataSize += pack->getLenghtDataBuffers();
                    lostSize += pack->getLostAll();
                }
                if (m_verbMode) {
                    timeNow = std::chrono::system_clock::now();
                    curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
                    value = curTime.time_since_epoch();

                    if ((value.count() - timeBegin) >= 5000) {
                        aprintf(stdout, "[ADC] Pass buffers: %d\n", m_passRate);
                        m_passRate = 0;
                        timeBegin = value.count();
                    }
                }
            }
        }
        auto timeNowEnd = std::chrono::system_clock::now();
        auto p1 = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow).time_since_epoch();
        auto p2 = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNowEnd).time_since_epoch();
        aprintf(stderr, "Loop %lld ms FPGA size %lld FPGA lost: %lld totalPassRate %lld\n", p2.count() - p1.count(), dataSize, lostSize, totalPassRate);
    } catch (std::exception& e) {
        aprintf(stderr, "Error: CStreamingApplication::oscWorker() %s\n", e.what());
    }
    setIsRun(false);
    unlockBuffF();
}

auto CStreamingFPGA::passCh() -> DataLib::CDataBuffersPackDMA::Ptr {
    int64_t time = m_Osc_ch->getBufferTime(m_currentBuffer);
    // The 64 bit counter of each channel, latched when this buffer started: the samples missing
    // right before it. The part not written while waiting for a trigger (the series gap of the
    // block mode) is a pause, not a loss: it goes apart, the loss keeps the overflow only.
    uint64_t wait = m_Osc_ch->getBufferWait(m_currentBuffer);
    uint64_t lost[4];
    for (uint8_t ch = 0; ch < 4; ch++) {
        uint64_t all = m_Osc_ch->getBufferLost(m_currentBuffer, ch);
        lost[ch] = all - std::min(all, wait);
    }

    if (m_printDebugBuffer) {
        exit(1);
    }

    if (!getBuffF || !unlockBuffF) {
        FATAL("Callback functions is not defined")
    }

    auto packNew = getBuffF();
    DataLib::CDataBuffersPackDMA::Ptr pack = nullptr;
    if (packNew) {
        if (!m_saveCaptureTime)
            time = 0;
        uint32_t triggerCount = m_Osc_ch->getTriggerCount();
        pack = m_mappedBuffers[m_currentBuffer];
        for (uint8_t ch = 0; ch < 4; ch++) {
            auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)ch);
            if (!buff)
                continue;
            // a dropped buffer is lost as a whole, its wait stays a pause
            buff->setLostSamples(DataLib::FPGA, lost[ch] + m_overFlowSumm[ch] + buff->getSamplesCount() * m_overFlowSummCount);
            buff->setLostSamples(DataLib::WAIT, wait + m_waitSumm);
            buff->setTimeCapture(time);
            buff->setDecimation(m_decimation);
            buff->setADCClock(m_adcClock);
            buff->setTriggerCount(triggerCount);
            buff->setTriggerBlocks(m_triggerBlocks);
            buff->setADCFlags(m_adcFlags);
        }

        for (auto& s : m_overFlowSumm)
            s = 0;
        m_overFlowSummCount = 0;
        m_waitSumm = 0;
        unlockBuffF();
        m_mappedBuffers[m_currentBuffer] = packNew;
        if (!m_testMode) {
            m_Osc_ch->setDataAddress(m_currentBuffer,
                                     packNew->getBufferDataAddress(DataLib::CH1),
                                     packNew->getBufferDataAddress(DataLib::CH2),
                                     packNew->getBufferDataAddress(DataLib::CH3),
                                     packNew->getBufferDataAddress(DataLib::CH4));
        }
    } else {
        for (uint8_t ch = 0; ch < 4; ch++)
            m_overFlowSumm[ch] += lost[ch];
        m_overFlowSummCount++;
        m_waitSumm += wait;
    }
    m_Osc_ch->clearBuffer(m_currentBuffer);
    m_currentBuffer = m_currentBuffer ? 0 : 1;
    return pack;
}

auto CStreamingFPGA::setTestMode(bool mode) -> void {
    m_testMode = mode;
}

auto CStreamingFPGA::setVerboseMode(bool mode) -> void {
    m_verbMode = mode;
}
