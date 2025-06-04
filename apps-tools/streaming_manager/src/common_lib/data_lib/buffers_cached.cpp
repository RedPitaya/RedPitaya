#include "buffers_cached.h"
#include <stdint.h>
#include "logger_lib/file_logger.h"
#include "logger_lib/profiler.h"
#include "network_header.h"

using namespace DataLib;

auto CBuffersCached::create() -> CBuffersCached::Ptr {
    return std::make_shared<CBuffersCached>();
}

CBuffersCached::CBuffersCached() : m_buffers(), m_needDestroy(false), m_dataSize(0) {}

CBuffersCached::~CBuffersCached() {
    std::lock_guard lock(m_mtx);
    notifyToDestory();
    m_buffers.clear();
    TRACE("Exit")
}

auto CBuffersCached::addChannel(DataLib::EDataBuffersPackChannel ch, uint8_t bits, DataLib::CDataBufferDMA::ADC_MODE _adc_mode) -> void {
    m_channels[ch] = bits;
    m_channelsMode[ch] = _adc_mode;
}

auto CBuffersCached::generateBuffers(std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize, bool testMode) -> void {
    if (m_channels.size() == 0) {
        ERROR_LOG("No active channels")
        return;
    }
    m_ringStart = 0;
    m_ringEnd = 0;
    m_ringSize = blocks.size() / m_channels.size();
    profiler::setTimePoint("initBuffer");
    size_t currentBlock = 0;
    for (auto i = 0u; i < m_ringSize; i++) {
        auto pack = DataLib::CDataBuffersPackDMA::Create();
        for (auto s : m_channels) {
            auto bits = s.second;
            auto block = blocks[currentBlock++];
            auto buff = DataLib::CDataBufferDMA::Create(block.start, block.size, block.startMemory, bits);
            buff->setADCMode(m_channelsMode[s.first]);
            if (headerSize) {
                buff->initHeaderAddress(headerSize);
                memset(buff->getMappedMemory(), 0, headerSize);
            }
            if (testMode) {
                uint8_t z = 0;
                uint8_t* data = static_cast<uint8_t*>(buff->getMappedDataMemory());
                for (uint32_t k = 0; k < buff->getDataLenght(); k++, z++) {
                    data[k] = z;
                }
            }
            pack->addBuffer(s.first, buff);
            m_dataSize = m_dataSize < buff->getDataLenght() ? buff->getDataLenght() : m_dataSize;
        }
        m_buffers.push_back(pack);
    }
    profiler::printuS("initBuffer", "Init buffer. Test mode %d", testMode);
    sem_init(&m_countsem, 0, 0);
    sem_init(&m_spacesem, 0, m_ringSize);
}

auto CBuffersCached::generateBuffersEmpty(uint32_t channels, std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize) -> void {
    if (channels == 0) {
        ERROR_LOG("No channels")
        return;
    }
    m_ringStart = 0;
    m_ringEnd = 0;
    m_ringSize = blocks.size() / channels;
    profiler::setTimePoint("initBuffer");
    size_t currentBlock = 0;
    for (auto i = 0u; i < m_ringSize; i++) {
        auto pack = DataLib::CDataBuffersPackDMA::Create();
        for (auto i = 0u; i < channels; i++) {
            auto bits = 8;
            auto block = blocks[currentBlock++];
            auto buff = DataLib::CDataBufferDMA::Create(block.start, block.size, block.startMemory, bits);
            if (headerSize) {
                buff->initHeaderAddress(headerSize);
                memset(buff->getMappedMemory(), 0, headerSize);
            }
            pack->addBuffer((EDataBuffersPackChannel)i, buff);
            m_dataSize = m_dataSize < buff->getDataLenght() ? buff->getDataLenght() : m_dataSize;
        }
        m_buffers.push_back(pack);
    }
    profiler::printuS("initBuffer", "Init buffer.");
    sem_init(&m_countsem, 0, 0);
    sem_init(&m_spacesem, 0, m_ringSize);
}

auto CBuffersCached::initHeadersADC() -> bool {
    std::lock_guard lock(m_mtx);
    for (size_t i = 0; i < m_buffers.size(); i++) {
        auto pack = m_buffers[i];
        for (auto ch = (int)DataLib::EDataBuffersPackChannel::CH1; ch <= (int)DataLib::EDataBuffersPackChannel::CH4; ch++) {
            auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)ch);
            if (buff != NULL) {
                DataLib::initHeaderADC(buff, buff->getADCBaseRate(), buff->getADCBaseBits(), pack->getLenghtDataBuffers(), ch);
            }
        }
    }
    return true;
}

auto CBuffersCached::initHeadersDAC(uint8_t channels) -> bool {
    std::lock_guard lock(m_mtx);
    for (size_t i = 0; i < m_buffers.size(); i++) {
        auto pack = m_buffers[i];
        for (auto ch = (int)DataLib::EDataBuffersPackChannel::CH1; ch <= (int)DataLib::EDataBuffersPackChannel::CH4; ch++) {
            auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)ch);
            if (buff != NULL) {
                DataLib::initHeaderDAC(buff, pack->getLenghtDataBuffers(), channels);
            }
        }
    }
    return true;
}

auto CBuffersCached::notifyToDestory() -> bool {
    m_needDestroy = true;
    return true;
}

auto CBuffersCached::isWaitToDestory() -> bool {
    return m_needDestroy;
}

auto CBuffersCached::isEmpty() -> bool {
    int sval;
    sem_getvalue(&m_countsem, &sval);
    return sval == 0;
}

inline auto CBuffersCached::getFreeSize() -> uint32_t {
    return m_ringSize - (m_ringEnd < m_ringStart ? ((m_ringEnd + m_ringSize) - m_ringStart) : (m_ringEnd - m_ringSize));
}

auto CBuffersCached::fullPercent() -> float {
    auto usedBuff = (m_ringEnd < m_ringStart ? ((m_ringEnd + m_ringSize) - m_ringStart) : (m_ringEnd - m_ringSize));
    return (float)usedBuff / (float)m_ringSize;
}

auto CBuffersCached::writeBuffer(bool timeout) -> DataLib::CDataBuffersPackDMA::Ptr {
    if (m_ringSize == 0) {
        return nullptr;
    }

    if (timeout) {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            return nullptr;
        }

        ts.tv_sec += 1;
        auto res = sem_timedwait(&m_spacesem, &ts);
        if (res != 0) {
			return nullptr;
		}
	} else {
        if (sem_wait(&m_spacesem) != 0) {
            return nullptr;
        }
    }

    m_mtx.lock();
    m_ringEnd = (m_ringEnd + 1) % m_ringSize;
    auto pack = m_buffers[m_ringEnd];
    m_mtx.unlock();
    return pack;
}

auto CBuffersCached::unlockBufferWrite() -> void {
    std::lock_guard lock(m_mtx);
    sem_post(&m_countsem);
}

auto CBuffersCached::unlockBufferRead() -> void {
    std::lock_guard lock(m_mtx);
    sem_post(&m_spacesem);
}

auto CBuffersCached::readBuffer() -> DataLib::CDataBuffersPackDMA::Ptr {
    if (m_ringSize == 0) {
        return nullptr;
    }

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return nullptr;
    }

    ts.tv_sec += 1;
    if (sem_timedwait(&m_countsem, &ts) != 0) {
        return nullptr;
    }

    m_mtx.lock();
    m_ringStart = (m_ringStart + 1) % m_ringSize;
    auto pack = m_buffers[m_ringStart];
    m_mtx.unlock();
    return pack;
}

auto CBuffersCached::getDataSize() -> uint32_t {
    return m_dataSize;
}

auto CBuffersCached::setOSCRate(uint64_t rate) -> void {
    std::lock_guard lock(m_mtx);
    for (size_t i = 0; i < m_buffers.size(); i++) {
        m_buffers[i]->setOSCRate(rate);
    }
}

auto CBuffersCached::setADCBits(uint8_t bits) -> void {
    std::lock_guard lock(m_mtx);
    for (size_t i = 0; i < m_buffers.size(); i++) {
        m_buffers[i]->setADCBits(bits);
    }
}
