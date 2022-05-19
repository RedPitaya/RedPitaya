#include <stdint.h>
#include "streaming_net_buffer.h"
#include "data_lib/thread_cout.h"
#include "data_lib/neon_asm.h"
#include "net_lib/asio_common.h"

using namespace streaming_lib;

auto CStreamingNetBuffer::create() -> CStreamingNetBuffer::Ptr{

    return std::make_shared<CStreamingNetBuffer>();
}

CStreamingNetBuffer::CStreamingNetBuffer():
    m_currentPack(nullptr),
    m_tempBuffer(),
    m_currentPackId(0),
    m_buffersAllSize(0)
{
}

CStreamingNetBuffer::~CStreamingNetBuffer()
{
}

auto CStreamingNetBuffer::addNewBuffer(uint8_t* buffer,size_t len) -> void {
    std::lock_guard<std::mutex> lock(m_mtx);
    uint64_t new_id = 0;
    size_t   buffersAllSize = 0;
    uint64_t packOrderId = 0;
    DataLib::EDataBuffersPackChannel channel = DataLib::EDataBuffersPackChannel::CH1;

    auto begPack = net_lib::extractBeginPack(buffer,len,&new_id,&buffersAllSize);
    if (begPack){
//        aprintf(stderr,"extractBeginPack %d cur %d\n",new_id,m_currentPackId);
        if (m_currentPack){
            // Drop broken buffer
            resetInternalBuffers();
            brokenPacksNotify(new_id - m_currentPackId);
            m_currentPack = begPack;
            m_currentPackId = new_id;
            m_buffersAllSize = buffersAllSize;
        }else{
            // Check buffers order and drop old
            if (m_currentPackId <= new_id){
                m_currentPack = begPack;
                m_currentPackId = new_id;
                m_buffersAllSize = buffersAllSize;
            }
        }
        return;
    }

    auto endPack = net_lib::extractEndPack(buffer,len,&new_id);
    if (endPack){
//        aprintf(stderr,"extractEndPack %d cur %d\n",new_id,m_currentPackId);
        if (m_currentPack){
            if (m_currentPackId == new_id){
                if (isAllData()){
                    for(auto &kv: m_tempBuffer){
                        auto new_buff = kv.second.convertBuffer();
                        if (new_buff){
                            m_currentPack->addBuffer(kv.first,new_buff);
                        }else{
                            resetInternalBuffers();
                            outMemoryNotify(1);
                            return;
                        }
                    }
                    receivedPackNotify(m_currentPack);
                    resetInternalBuffers();

                }else{
                    resetInternalBuffers();
                    brokenPacksNotify(1);
                    return;
                }
            }
        }
        return;
    }

    auto buffPack = net_lib::extractBufferPack(buffer,len,&new_id,&packOrderId,&channel);
    if (buffPack){
//        aprintf(stderr,"extractBufferPack %d cur %d\n",new_id , m_currentPackId);
        if (new_id == m_currentPackId){
            if (m_tempBuffer.find(channel) == m_tempBuffer.end()){
                m_tempBuffer[channel] = BuffersAgregator();
            }
            m_tempBuffer[channel].buff_map[packOrderId] = buffPack;
        }
        return;
    }
}

auto CStreamingNetBuffer::resetInternalBuffers() -> void{
    m_currentPack = nullptr;
    m_tempBuffer.clear();
    m_buffersAllSize = 0;
}


auto CStreamingNetBuffer::BuffersAgregator::getBuffersLenght() -> uint64_t {
    uint64_t size = 0;
    for(auto &kv : buff_map){
        size += kv.second->getBufferLenght();
    }
    return size;
}

auto CStreamingNetBuffer::BuffersAgregator::convertBuffer() -> DataLib::CDataBuffer::Ptr{
    try{
        size_t buf_len = getBuffersLenght();
        auto buf = net_lib::createBuffer(buf_len);
        if (!buf){
            return nullptr;
        }

        uint64_t packs = buff_map.size();
        if (packs){
            uint64_t position = 0;
            for(uint64_t id = 0; id < packs; id++){
                memcpy_neon(buf.get() + position,buff_map[id]->getBuffer().get(),buff_map[id]->getBufferLenght());
                position += buff_map[id]->getBufferLenght();
            }
            auto ch_buffer = DataLib::CDataBuffer::Create(buf,buf_len,buff_map[0]->getBitBySample());
            ch_buffer->setADCMode(buff_map[0]->getADCMode());
            ch_buffer->setLostSamples(DataLib::FPGA,buff_map[0]->getLostSamples(DataLib::FPGA));
            ch_buffer->setLostSamples(DataLib::RP_INTERNAL_BUFFER,buff_map[0]->getLostSamples(DataLib::RP_INTERNAL_BUFFER));
            return ch_buffer;
        }
        return nullptr;
    }catch(std::exception &ex){
        aprintf(stderr,"[FATAL ERROR] auto CStreamingNetBuffer::BuffersAgregator::convertBuffer() %s\n",ex.what());
        return nullptr;
    }
}


auto CStreamingNetBuffer::isAllData() -> bool{
    uint64_t allSize = 0;
    for(auto &kv : m_tempBuffer){
        allSize += kv.second.getBuffersLenght();
    }
    return allSize == m_buffersAllSize;
}
