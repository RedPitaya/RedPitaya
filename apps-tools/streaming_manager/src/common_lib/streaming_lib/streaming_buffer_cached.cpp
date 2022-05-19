#include <stdint.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>
#include "streaming_buffer_cached.h"
#include "data_lib/thread_cout.h"

using namespace streaming_lib;

auto CStreamingBufferCached::create(uint32_t maxRamSize) -> CStreamingBufferCached::Ptr{

    return std::make_shared<CStreamingBufferCached>(maxRamSize);
}

CStreamingBufferCached::CStreamingBufferCached(uint32_t maxRamSize) :
    m_buffers(),
    m_maxRamSize(0),
//    m_currentRamSize(0),
    m_needDestroy(false),
    m_dropedPack(nullptr)
{
    setMaxRamSize(maxRamSize);
}

CStreamingBufferCached::~CStreamingBufferCached()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    notifyToDestory();    
    m_buffers.clear();
}

auto CStreamingBufferCached::addChannel(DataLib::EDataBuffersPackChannel ch,size_t size,uint8_t bitBySample) -> void{
    m_channelsSize[ch] = {size,bitBySample};
}
    

auto CStreamingBufferCached::generateBuffers() -> void{
    auto allSize = 0;
    m_ringStart = 0;
    m_ringEnd = 0;
    m_ringSize = 0;
    for(auto s:m_channelsSize){
        allSize += s.second.first;
    }

    for(auto curSize = 0u; curSize < m_maxRamSize; curSize += allSize){
        auto pack = DataLib::CDataBuffersPack::Create();
        for(auto s:m_channelsSize){
            auto osclen = s.second.first;
            auto bits = s.second.second;
            auto buffLen =  bits == 8 ? osclen / 2 : osclen;
            auto buff = DataLib::CDataBuffer::Create(std::shared_ptr<uint8_t[]>(new uint8_t[osclen]),buffLen, bits);
            buff->recalcBufferLenght();
            pack->addBuffer(s.first,buff);
        }
        m_buffers.push_back(pack);
        m_ringSize++;
    }
}

auto CStreamingBufferCached::getMaxRamSize() -> uint64_t{
    return m_maxRamSize;
}

auto CStreamingBufferCached::setMaxRamSize(uint64_t size) -> void{
    if (m_maxRamSize != size){
        m_maxRamSize = size;
        maxRamNotify(m_maxRamSize);
    }
}

auto CStreamingBufferCached::notifyToDestory() -> bool{
    m_needDestroy = true;
    return true;
}

auto CStreamingBufferCached::isWaitToDestory() -> bool{
    return m_needDestroy;
}

inline auto CStreamingBufferCached::getFreeSize() -> uint32_t{
    return m_ringSize - (m_ringEnd < m_ringStart ? ((m_ringEnd + m_ringSize) - m_ringStart) : (m_ringEnd - m_ringSize));
}

auto CStreamingBufferCached::fullPercent() -> float{
    auto usedBuff = (m_ringEnd < m_ringStart ? ((m_ringEnd + m_ringSize) - m_ringStart) : (m_ringEnd - m_ringSize));
    return (float)usedBuff / (float)m_ringSize;
}

auto CStreamingBufferCached::getFreeBuffer(uint64_t fpga_lost) -> DataLib::CDataBuffersPack::Ptr{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (((m_ringEnd + 1) % m_ringSize) != m_ringStart){
        auto pack = m_buffers[m_ringEnd];
        return pack;
    }else{
        for(int i = (int)DataLib::CH1; i < (int)DataLib::CH4; i++){
            auto pack = m_buffers[m_ringEnd];
            auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
            if (buff){
                // increase lost data by one buffer + fpga lost
                buff->setLostSamples(DataLib::RP_INTERNAL_BUFFER,buff->getLostSamples(DataLib::RP_INTERNAL_BUFFER) + buff->getSamplesCount() + fpga_lost);
            }
        }
    }
    return nullptr;
}

auto CStreamingBufferCached::unlockBufferWrite() -> void{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_ringEnd = (m_ringEnd + 1) % m_ringSize;
}

auto CStreamingBufferCached::unlockBufferRead() -> void{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_ringStart = (m_ringStart + 1) % m_ringSize;
}

auto CStreamingBufferCached::readBuffer() -> DataLib::CDataBuffersPack::Ptr{
    std::lock_guard<std::mutex> lock(m_mtx);    
    if (m_ringStart != m_ringEnd){
        auto pack = m_buffers[m_ringStart];
        return pack;
    }
    return nullptr;
}