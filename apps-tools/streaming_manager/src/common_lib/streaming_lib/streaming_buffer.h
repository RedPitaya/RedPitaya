#ifndef STREAMING_LIB_STREAMING_BUFFER_H
#define STREAMING_LIB_STREAMING_BUFFER_H

#include <mutex>
#include <list>
#include <deque>

#include "data_lib/signal.hpp"
#include "data_lib/buffers_pack.h"

namespace streaming_lib {

class CStreamingBuffer
{
public:

    using Ptr = std::shared_ptr<CStreamingBuffer>;
    
    static auto create(uint32_t maxRamSize = 1024 * 1024 * 50) -> Ptr;

    CStreamingBuffer(uint32_t maxRamSize);
    ~CStreamingBuffer();
    
    auto addNewBuffer(DataLib::CDataBuffersPack::Ptr pack) -> void;
    auto getBuffer() -> DataLib::CDataBuffersPack::Ptr;

    auto getCurrentRamSize() -> uint64_t;
    auto getMaxRamSize() -> uint64_t;
    auto setMaxRamSize(uint64_t size) -> void;
    auto isBufferFull() -> bool;
    auto fullProcent() -> float;
    auto notifyToDestory() -> bool;
    auto isWaitToDestory() -> bool;

    sigslot::signal<uint64_t> maxRamNotify;
    sigslot::signal<> packDropNotify;
    sigslot::signal<> bufferFullNotify;
    sigslot::signal<uint64_t> currentRamSizeNotify;

private:

    CStreamingBuffer(const CStreamingBuffer &) = delete;
    CStreamingBuffer(CStreamingBuffer &&) = delete;
    CStreamingBuffer& operator=(const CStreamingBuffer&) =delete;
    CStreamingBuffer& operator=(const CStreamingBuffer&&) =delete;

    auto addCurrentRamSize(uint64_t size) -> void;
    auto subCurrentRamSize(uint64_t size) -> void;

    std::deque<DataLib::CDataBuffersPack::Ptr> m_buffers;
    uint64_t m_maxRamSize;
    uint64_t m_currentRamSize;
    bool     m_needDestroy;
    DataLib::CDataBuffersPack::Ptr m_dropedPack;
    std::mutex m_mtx;
};

}

#endif
