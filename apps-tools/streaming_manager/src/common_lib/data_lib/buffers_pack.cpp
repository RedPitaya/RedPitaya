#include <iostream>
#include "buffers_pack.h"
#include "neon_asm.h"
#include "thread_cout.h"

using namespace DataLib;

auto CDataBuffersPack::Create() -> CDataBuffersPack::Ptr{
    return std::make_shared<CDataBuffersPack>();
}

CDataBuffersPack::CDataBuffersPack():
     m_buffers()
    ,m_oscRate(0)
    ,m_adc_bits(0)
{
}

CDataBuffersPack::~CDataBuffersPack(){
}

auto CDataBuffersPack::addBuffer(EDataBuffersPackChannel channel,DataLib::CDataBuffer::Ptr buffer) -> void{
    m_buffers[channel] = buffer;
}

auto CDataBuffersPack::getBuffer(EDataBuffersPackChannel channel) const -> DataLib::CDataBuffer::Ptr{
    if (m_buffers.count(channel)){
        return m_buffers.at(channel);
    }
    return nullptr;
}

auto CDataBuffersPack::isChannelPresent(EDataBuffersPackChannel channel) -> bool{
    return m_buffers.find(channel) != m_buffers.end();
}

auto CDataBuffersPack::setOSCRate(uint64_t rate) -> void{
    m_oscRate = rate;
}

auto CDataBuffersPack::getOSCRate() -> uint64_t{
    return m_oscRate;
}

auto CDataBuffersPack::setADCBits(uint8_t bits) -> void{
    m_adc_bits = bits;
}

auto CDataBuffersPack::getADCBits() -> uint8_t{
    return m_adc_bits;
}

auto CDataBuffersPack::checkBuffersEqual() -> bool{
    size_t size = 0;
    uint8_t bits = 0;
    for (const auto& kv : m_buffers) {
        auto buff_size = kv.second->getBufferLenght();
        auto buff_bit = kv.second->getBitBySample();
        if (buff_size){
               if (size != 0){
                   if (size != buff_size){
                       return false;
                   }
               }else{
                   size = buff_size;
               }
        }
        if (buff_bit){
               if (bits != 0){
                   if (bits != buff_bit){
                       return false;
                   }
               }else{
                   bits = buff_bit;
               }
        }
    }
    return true;
}

auto CDataBuffersPack::getBuffersLenght() -> size_t{
    size_t size = 0;
    for (const auto& kv : m_buffers) {
        if (kv.second->getBufferLenght()){
            if (size < kv.second->getBufferLenght()){
                if (size > 0){
                    aprintf(stderr,"[WARNING] The buffers are not the same length. The program may not work correctly.\n");
                }
                size = kv.second->getBufferLenght();
            }
        }
    }
    return size;
}

auto CDataBuffersPack::getLenghtAllBuffers() -> uint64_t{
    uint64_t size = 0;
    for (const auto& kv : m_buffers) {
        size += kv.second->getBufferLenght();
    }
    return size;
}

auto CDataBuffersPack::getLostAllBuffers() -> uint64_t{
    uint64_t size = 0;
    for (const auto& kv : m_buffers) {
        size += kv.second->getLostSamplesInBytesLenght();
    }
    return size;
}


auto CDataBuffersPack::getBuffersSamples() -> size_t{
    size_t size = 0;
    for (const auto& kv : m_buffers) {
        if (kv.second->getSamplesCount()){
            if (size < kv.second->getSamplesCount()){
                size = kv.second->getSamplesCount();
            }
        }
    }
    return size;
}
