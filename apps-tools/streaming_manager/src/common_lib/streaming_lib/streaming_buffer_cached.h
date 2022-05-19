#ifndef STREAMING_LIB_STREAMING_BUFFER_CACHED_H
#define STREAMING_LIB_STREAMING_BUFFER_CACHED_H

#include <mutex>
#include <list>
#include <deque>
#include <map>


#include "data_lib/signal.hpp"
#include "data_lib/buffers_pack.h"

namespace streaming_lib {

class CStreamingBufferCached
{
public:

    using Ptr = std::shared_ptr<CStreamingBufferCached>;
    
    static auto create(uint32_t maxRamSize = 1024 * 1024 * 50) -> Ptr;
    
    CStreamingBufferCached(uint32_t maxRamSize);
    ~CStreamingBufferCached();
    
    auto addChannel(DataLib::EDataBuffersPackChannel ch,size_t size,uint8_t bitBySample) -> void;
        
    auto generateBuffers() -> void;

    auto getFreeBuffer(uint64_t fpga_lost) -> DataLib::CDataBuffersPack::Ptr;
    auto unlockBufferWrite() -> void;
    auto unlockBufferRead() -> void;
    auto readBuffer() -> DataLib::CDataBuffersPack::Ptr;

    auto getMaxRamSize() -> uint64_t;
    auto setMaxRamSize(uint64_t size) -> void;
    auto fullPercent() -> float;
    auto notifyToDestory() -> bool;
    auto isWaitToDestory() -> bool;

    sigslot::signal<uint64_t> maxRamNotify;
    sigslot::signal<> packDropNotify;
    sigslot::signal<> bufferFullNotify;
    sigslot::signal<uint64_t> currentRamSizeNotify;

private:

    CStreamingBufferCached(const CStreamingBufferCached &) = delete;
    CStreamingBufferCached(CStreamingBufferCached &&) = delete;
    CStreamingBufferCached& operator=(const CStreamingBufferCached&) =delete;
    CStreamingBufferCached& operator=(const CStreamingBufferCached&&) =delete;

    auto getFreeSize() -> uint32_t;

    std::vector<DataLib::CDataBuffersPack::Ptr> m_buffers;
    uint32_t m_ringStart;
    uint32_t m_ringEnd;
    uint32_t m_ringSize;

    std::map<DataLib::EDataBuffersPackChannel,std::pair<size_t,uint8_t>> m_channelsSize;
    uint64_t m_maxRamSize;
    bool     m_needDestroy;
    DataLib::CDataBuffersPack::Ptr m_dropedPack;
    std::mutex m_mtx;
};

}

#endif
