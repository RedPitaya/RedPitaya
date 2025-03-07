#include "dac_streaming_application.h"
#include "logger_lib/file_logger.h"

using namespace dac_streaming_lib;

CDACStreamingApplication::CDACStreamingApplication(CDACStreamingManager::Ptr _streamingManager, uio_lib::CGenerator::Ptr _gen)
    : m_gen(_gen), m_streamingManager(_streamingManager), m_Thread(), mtx(), m_ReadyToPass(0), m_isRun(false), m_isRunNonBloking(false) {}

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
            buffer = !onePackMode || buffer == nullptr ? m_streamingManager->getBuffer() : buffer;
            if (buffer) {

                auto ch1 = buffer->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
                auto ch2 = buffer->getBuffer(DataLib::EDataBuffersPackChannel::CH2);
                uint32_t ch1Address = 0;
                uint32_t ch2Address = 0;
                uint32_t chSize = 0;
                bool ch18Bit = false;
                bool ch28Bit = false;
                if (ch1) {
                    ch1Address = ch1->getDataAddress();
                    if (ch1->getDACOnePackMode()) {
                        chSize = std::max(ch1->getDACChannelSize(), chSize);
                        repeatInOpenPackMode = std::max(repeatInOpenPackMode, ch1->getDACRepeatCount());
                        one_pack_inf_mode = ch1->getDACInfMode();
                        ch1->decDACRepeatCount();
                        ch18Bit = ch1->getDACBits() == 8;
                        onePackMode = true;
                    } else {
                        chSize = std::max((uint32_t)ch1->getDataLenght(), chSize);
                    }
                }

                if (ch2) {
                    ch2Address = ch2->getDataAddress();
                    if (ch2->getDACOnePackMode()) {
                        chSize = std::max(ch2->getDACChannelSize(), chSize);
                        repeatInOpenPackMode = std::max(repeatInOpenPackMode, ch2->getDACRepeatCount());
                        one_pack_inf_mode = ch2->getDACInfMode();
                        ch2->decDACRepeatCount();
                        ch28Bit = ch2->getDACBits() == 8;
                        onePackMode = true;
                    } else {
                        chSize = std::max((uint32_t)ch2->getDataLenght(), chSize);
                    }
                }

                if (onePackMode && repeatInOpenPackMode == 0 && one_pack_inf_mode == false) {
                    break;
                }
                // buffer->debugPackDAC();
                bool ret = false;
                do {
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
            }
        }
        if (onePackMode) {
            m_streamingManager->unlockBuffer();
        }
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what())
    }
    m_gen->stop();
    m_isRun = false;
}

void CDACStreamingApplication::signalHandler(const std::error_code&, int) {
    stop();
}
