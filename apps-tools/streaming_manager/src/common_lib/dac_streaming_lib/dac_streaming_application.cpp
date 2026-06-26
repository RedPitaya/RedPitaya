#include "dac_streaming_application.h"
#include "logger_lib/file_logger.h"

using namespace dac_streaming_lib;

CDACStreamingApplication::CDACStreamingApplication(CDACStreamingManager::Ptr _streamingManager, uio_lib::CGenerator::Ptr _gen)
    : m_gen(_gen), m_streamingManager(_streamingManager), m_Thread(), mtx(), m_ReadyToPass(0), m_isRun(false), m_isRunNonBloking(false), m_verbMode(false) {}

CDACStreamingApplication::~CDACStreamingApplication() {
    stop();
}

auto CDACStreamingApplication::run() -> void {
    m_isRun = true;
    m_isRunNonBloking = false;
    try {
        m_streamingManager->run();
        m_Thread = std::thread(&CDACStreamingApplication::genWorker, this);
        if (m_Thread.joinable()) {
            m_Thread.join();
        }
    } catch (const std::exception& e) {
        ERROR_LOG("%s", e.what())
    }
}

auto CDACStreamingApplication::runNonBlock() -> void {
    m_isRun = true;
    m_isRunNonBloking = true;
    try {
        m_streamingManager->run();  // MUST BE INIT FIRST for thread logic
        m_Thread = std::thread(&CDACStreamingApplication::genWorker, this);
    } catch (const std::exception& e) {
        ERROR_LOG("%s", e.what())
    }
}

auto CDACStreamingApplication::stop() -> bool {
    std::lock_guard lock(mtx);
    m_GenThreadRun = false;
    if (m_Thread.joinable()) {
        m_Thread.join();
    }
    if (m_streamingManager)
        m_streamingManager->stop();
    m_streamingManager = nullptr;
    if (m_gen)
        m_gen->stop();
    m_gen = nullptr;
    return true;
}

void CDACStreamingApplication::genWorker() {
    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
    auto value = curTime.time_since_epoch();
    long long int timeBegin = value.count();

    int indexForWrite = 0;
    DataLib::CDataBuffersPackDMA::Ptr buffer = nullptr;
    bool onePackMode = false;
    bool notRun = true;
    m_gen->stop();
    m_gen->prepare();
    m_GenThreadRun = true;
    try {
        while (m_GenThreadRun) {
            int64_t repeatInOpenPackMode = 0;
            bool one_pack_inf_mode = false;
            buffer = (!onePackMode || buffer == nullptr) ? m_streamingManager->getBuffer() : buffer;
            if (buffer) {
                auto ch1 = buffer->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
                auto ch2 = buffer->getBuffer(DataLib::EDataBuffersPackChannel::CH2);
                uint32_t ch1Address = 0;
                uint32_t ch2Address = 0;
                uint32_t chSize = 0;
                bool ch18Bit = false;
                bool ch28Bit = false;
                if (ch1 && ch1->getDACChannelSize()) {
                    ch1Address = ch1->getDataAddress();
                    if (ch1->getDACOnePackMode()) {
                        chSize = std::max(ch1->getDACChannelSize(), chSize);
                        repeatInOpenPackMode = std::max(repeatInOpenPackMode, ch1->getDACRepeatCount());
                        one_pack_inf_mode = ch1->getDACInfMode();
                        ch1->decDACRepeatCount();
                        ch18Bit = ch1->getDACBits() == 8;
                        onePackMode = true;
                    } else {
                        ch18Bit = ch1->getDACBits() == 8;
                        chSize = std::max((uint32_t)ch1->getDACChannelSize(), chSize);
                    }
                }

                if (ch2 && ch2->getDACChannelSize()) {
                    ch2Address = ch2->getDataAddress();
                    if (ch2->getDACOnePackMode()) {
                        chSize = std::max(ch2->getDACChannelSize(), chSize);
                        repeatInOpenPackMode = std::max(repeatInOpenPackMode, ch2->getDACRepeatCount());
                        one_pack_inf_mode = ch2->getDACInfMode();
                        ch2->decDACRepeatCount();
                        ch28Bit = ch2->getDACBits() == 8;
                        onePackMode = true;
                    } else {
                        ch28Bit = ch2->getDACBits() == 8;
                        chSize = std::max((uint32_t)ch2->getDACChannelSize(), chSize);
                    }
                }

                if (onePackMode && repeatInOpenPackMode == 0 && one_pack_inf_mode == false) {
                    break;
                }
                // buffer->debugPackDAC();
                bool ret = false;
                do {
                    // We are waiting for the free conveyor to be freed up in FPGA
                    ret = m_gen->setDataAddress(indexForWrite, ch1Address, ch2Address, chSize);
                    if (notRun) {
                        m_gen->setDataBits(ch18Bit, ch28Bit);
                        m_gen->start(ch1Address, ch2Address);
                        notRun = false;
                    }
                    if (!m_GenThreadRun)
                        break;
                } while (!ret);
                indexForWrite = indexForWrite == 0 ? 1 : 0;

                if (!onePackMode) {
                    m_streamingManager->unlockBuffer();
                }
                if (m_verbMode) {
                    timeNow = std::chrono::system_clock::now();
                    curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
                    value = curTime.time_since_epoch();

                    if ((value.count() - timeBegin) >= 5000) {
                        auto bufferManager = m_streamingManager->getBufferManager();
                        if (bufferManager) {
                            aprintf(stdout, "[DAC] Buffer status : %.2f%%\n", bufferManager->fullPercent() * 100.f);
                        }
                        timeBegin = value.count();
                    }
                }
            }
        }
        if (onePackMode) {
            m_streamingManager->unlockBuffer();
        }
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what())
    }
    m_gen->stop();
    if (!m_streamingManager->isRunned())  // Send notification that generation is complete at application level
        m_streamingManager->notifyStop(CDACStreamingManager::NR_ENDED);
    m_isRun = false;
}

void CDACStreamingApplication::signalHandler(const std::error_code&, int) {
    stop();
}

auto CDACStreamingApplication::setVerboseMode(bool mode) -> void {
    m_verbMode = mode;
}