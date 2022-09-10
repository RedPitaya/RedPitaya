#include <stdint.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>
#include "streaming_buffer.h"
#include "data_lib/thread_cout.h"

using namespace streaming_lib;

auto CStreamingBuffer::create(uint32_t maxRamSize) -> CStreamingBuffer::Ptr{

    return std::make_shared<CStreamingBuffer>(maxRamSize);
}

CStreamingBuffer::CStreamingBuffer(uint32_t maxRamSize) :
    m_buffers(),
    m_maxRamSize(0),
    m_currentRamSize(0),
    m_needDestroy(false),
    m_dropedPack(nullptr)
{
    setMaxRamSize(maxRamSize);
}

CStreamingBuffer::~CStreamingBuffer()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    notifyToDestory();    
    m_buffers.clear();
}

auto CStreamingBuffer::getMaxRamSize() -> uint64_t{
    return m_maxRamSize;
}

auto CStreamingBuffer::setMaxRamSize(uint64_t size) -> void{
    if (m_maxRamSize != size){
        m_maxRamSize = size;
        maxRamNotify(m_maxRamSize);
    }
}

auto CStreamingBuffer::notifyToDestory() -> bool{
    m_needDestroy = true;
    return true;
}

auto CStreamingBuffer::isWaitToDestory() -> bool{
    return m_needDestroy;
}

auto CStreamingBuffer::isBufferFull() -> bool{
    return m_maxRamSize < m_currentRamSize;
}

auto CStreamingBuffer::fullProcent() -> float{
    return (float)m_currentRamSize / (float)m_maxRamSize;
}

auto CStreamingBuffer::addCurrentRamSize(uint64_t size) -> void{
    auto isfb = isBufferFull();
    m_currentRamSize += size;
    if (isfb != isBufferFull()){
        bufferFullNotify();
    }
    currentRamSizeNotify(m_currentRamSize);
}

auto CStreamingBuffer::subCurrentRamSize(uint64_t size) -> void{
    auto isfb = isBufferFull();
    if (m_currentRamSize < size){
        m_currentRamSize = 0;
    }else{
        m_currentRamSize -= size;
    }
    if (isfb != isBufferFull()){
        bufferFullNotify();
    }
    currentRamSizeNotify(m_currentRamSize);
}

auto CStreamingBuffer::getCurrentRamSize() -> uint64_t{
    return m_currentRamSize;
}

auto CStreamingBuffer::addNewBuffer(DataLib::CDataBuffersPack::Ptr pack) -> void{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (isBufferFull()){

        if (m_dropedPack){
            for(int i = (int)DataLib::CH1; i < (int)DataLib::CH4; i++){
                auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
                auto buffDest = m_dropedPack->getBuffer((DataLib::EDataBuffersPackChannel)i);
                if (buff && buffDest){
                    buffDest->setLostSamples(DataLib::RP_INTERNAL_BUFFER,buffDest->getLostSamples(DataLib::RP_INTERNAL_BUFFER) + buff->getLostSamples(DataLib::FPGA) + buff->getSamplesCount());
                }
            }
        }else{
            m_dropedPack = pack;
        }
        packDropNotify();
    }else{
        if (m_dropedPack){
            m_buffers.push_back(m_dropedPack);
            auto size = sizeof(DataLib::CDataBuffersPack) + m_dropedPack->getLenghtAllBuffers();
            addCurrentRamSize(size);
            m_dropedPack = nullptr;
        }
        auto size = sizeof(DataLib::CDataBuffersPack) + pack->getLenghtAllBuffers();
        addCurrentRamSize(size);
        m_buffers.push_back(pack);
    }
}

auto CStreamingBuffer::getBuffer() -> DataLib::CDataBuffersPack::Ptr{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (!m_buffers.empty()){
        auto pack = m_buffers.front();
        m_buffers.pop_front();
        auto size = sizeof(DataLib::CDataBuffersPack) + pack->getLenghtAllBuffers();
        subCurrentRamSize(size);
        return pack;
    }else{
        if (m_dropedPack){
            auto tmp = m_dropedPack;
            m_dropedPack = nullptr;
            return tmp;
        }
    }
    return nullptr;
}


