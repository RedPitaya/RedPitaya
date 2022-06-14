#ifndef STREAMING_LIB_STREAMING_NET_BUFFER_H
#define STREAMING_LIB_STREAMING_NET_BUFFER_H

#include <mutex>
#include <list>

#include "data_lib/signal.hpp"
#include "data_lib/buffers_pack.h"

namespace streaming_lib {

class CStreamingNetBuffer
{
public:

    using Ptr = std::shared_ptr<CStreamingNetBuffer>;
    
    static auto create() -> Ptr;

    CStreamingNetBuffer();
    ~CStreamingNetBuffer();
    
    auto addNewBuffer(uint8_t* buffer,size_t len) -> void;

//    auto getCurrentRamSize() -> uint64_t;
//    auto getMaxRamSize() -> uint64_t;
//    auto setMaxRamSize(uint64_t size) -> void;
//    auto isBufferFull() -> bool;


    sigslot::signal<uint64_t> brokenPacksNotify;
    sigslot::signal<uint64_t> outMemoryNotify;
    sigslot::signal<DataLib::CDataBuffersPack::Ptr,uint64_t> receivedPackNotify;


//    sigslot::signal<> packDropNotify;
//    sigslot::signal<> bufferFullNotify;
//    sigslot::signal<uint64_t> currentRamSizeNotify;

private:

    struct BuffersAgregator{
        std::map<uint64_t,DataLib::CDataBuffer::Ptr> buff_map;
        auto getBuffersLenght() -> uint64_t;
        auto convertBuffer() -> DataLib::CDataBuffer::Ptr;
    };

    CStreamingNetBuffer(const CStreamingNetBuffer &) = delete;
    CStreamingNetBuffer(CStreamingNetBuffer &&) = delete;
    CStreamingNetBuffer& operator=(const CStreamingNetBuffer&) =delete;
    CStreamingNetBuffer& operator=(const CStreamingNetBuffer&&) =delete;

    auto resetInternalBuffers() -> void;
    auto isAllData() -> bool;

    DataLib::CDataBuffersPack::Ptr m_currentPack;
    std::map<DataLib::EDataBuffersPackChannel,BuffersAgregator> m_tempBuffer;
    uint64_t m_currentPackId;
    size_t   m_buffersAllSize;
    std::mutex m_mtx;
};

}

#endif
