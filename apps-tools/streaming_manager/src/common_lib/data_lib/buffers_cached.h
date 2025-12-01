#ifndef DATA_LIB_BUFFERS_CACHED_H
#define DATA_LIB_BUFFERS_CACHED_H

#include <semaphore.h>
#include <map>
#include <mutex>

#include "buffers_pack.h"
#include "uio_lib/memory_manager.h"

namespace DataLib {

class CBuffersCached {
   public:
    using Ptr = std::shared_ptr<CBuffersCached>;

    static auto create() -> Ptr;

    CBuffersCached();
    ~CBuffersCached();

    auto addChannel(DataLib::EDataBuffersPackChannel ch, uint8_t bits, DataLib::CDataBufferDMA::ADC_MODE _adc_mode) -> void;
    auto generateBuffers(std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize = 0, bool testMode = false) -> void;
    auto generateBuffersEmpty(uint32_t channels, std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize = 0) -> void;

    auto writeBuffer(bool timeout = false) -> DataLib::CDataBuffersPackDMA::Ptr;
    auto unlockBufferWrite() -> void;

    auto readBuffer() -> DataLib::CDataBuffersPackDMA::Ptr;
    auto unlockBufferRead() -> void;

    auto initHeadersADC() -> bool;
    auto initHeadersDAC(uint8_t channels) -> bool;

    auto fullPercent() -> float;
    auto notifyToDestory() -> bool;
    auto isWaitToDestory() -> bool;
    auto getDataSize() -> uint32_t;
    auto isEmpty() -> bool;

    auto setOSCRate(uint64_t rate) -> void;
    auto setADCBits(uint8_t bits) -> void;

   private:
    CBuffersCached(const CBuffersCached&) = delete;
    CBuffersCached(CBuffersCached&&) = delete;
    CBuffersCached& operator=(const CBuffersCached&) = delete;
    CBuffersCached& operator=(const CBuffersCached&&) = delete;

    auto getFreeSize() -> uint32_t;

    std::vector<DataLib::CDataBuffersPackDMA::Ptr> m_buffers;

    uint32_t m_ringStart;
    uint32_t m_ringEnd;
    uint32_t m_ringSize;

    std::map<DataLib::EDataBuffersPackChannel, uint8_t> m_channels;
    std::map<DataLib::EDataBuffersPackChannel, DataLib::CDataBufferDMA::ADC_MODE> m_channelsMode;
    bool m_needDestroy;
    std::mutex m_mtx;
    sem_t m_countsem;
    sem_t m_spacesem;
    uint32_t m_dataSize;
};

}  // namespace DataLib

#endif
