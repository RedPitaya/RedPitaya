#include <cstring>
#include "asio_common.h"
#include "data_lib/neon_asm.h"
#include "data_lib/thread_cout.h"

#define UNUSED(x) [&x]{}()

using namespace net_lib;

constexpr uint8_t net_lib::ID_PACK[]        = {0xFF,0xFF,0xFF,0xFF,0xA0,0xA0,0xA0,0xA0,0xFF,0xFF,0xFF,0xFF,0xA0,0xA0,0xA0,0xA0};
constexpr uint8_t net_lib::ID_PACK_END[]    = {0xFF,0xFF,0xFF,0xFF,0x50,0x50,0x50,0x50,0xFF,0xFF,0xFF,0xFF,0x50,0x50,0x50,0x50};
constexpr uint8_t net_lib::ID_BUFFER[]      = {0xFF,0xFF,0xFF,0xFF,0x0A,0x0A,0x0A,0x0A,0xFF,0xFF,0xFF,0xFF,0x0A,0x0A,0x0A,0x0A};

auto net_lib::createBuffer(const char *buffer,size_t size) -> net_buffer{
    try{
        auto p = std::shared_ptr<uint8_t[]>(new uint8_t[size]);
        memcpy_neon(p.get(),buffer,size);
        return p;
    } catch (const std::bad_alloc& e) {
        return nullptr;
    }
}

auto net_lib::createBuffer(uint64_t size) -> net_buffer{
    try{
        auto p = std::shared_ptr<uint8_t[]>(new uint8_t[size]);
        return p;
    } catch (const std::bad_alloc& e) {
        return nullptr;
    }
}

auto createBeginPack(uint64_t _id,DataLib::CDataBuffersPack::Ptr pack) -> AsioBufferNolder{
    AsioBufferNolder bh;
    bh.headerLen = 0;
    bh.dataPtr = nullptr;
    bh.dataLen = 0;
    bh.buffPackOwner = nullptr;

    try{
        uint64_t buffer_lenght = sizeof(int8_t) * 16; // ID of pack (16 byte)
        uint64_t packId = _id;
        uint64_t oscRate = pack->getOSCRate();
        uint64_t adcBits = pack->getADCBits();
        uint64_t buffersSize = pack->getLenghtAllBuffers();

        buffer_lenght += sizeof(uint64_t) * 5;
        // auto buff = std::shared_ptr<uint8_t[]>(new uint8_t[buffer_lenght]);
        // memcpy_neon(buff.get() ,net_lib::ID_PACK,16);
        memcpy_neon(bh.header,net_lib::ID_PACK,16);
        uint64_t* buff64 = reinterpret_cast<uint64_t*>(bh.header);
        buff64[2] = buffer_lenght; // FULL pack lenght
        buff64[3] = packId;
        buff64[4] = oscRate;
        buff64[5] = adcBits;
        buff64[6] = buffersSize;          
        bh.headerLen = buffer_lenght;
        return bh;
    } catch (const std::bad_alloc& e) {
        return bh;
    }
}

auto net_lib::extractBeginPack(uint8_t* _buffer,size_t _length,uint64_t *_id,size_t *_allBuffersSize) -> DataLib::CDataBuffersPack::Ptr{
    if (_length < 20){ // ID + buff_size attribute
        return  nullptr;
    }

    for(size_t i = 0 ;i < _length && i < 16; i++){
        if (net_lib::ID_PACK[i] != _buffer[i]){
            return nullptr;
        }
    }

    uint64_t* buff64    = reinterpret_cast<uint64_t*>(_buffer);
    uint64_t  buff_size = buff64[2];
    if (buff_size > _length){
        return nullptr;
    }
    uint64_t packId       = buff64[3];
    uint64_t oscRate      = buff64[4];
    uint64_t adcBits      = buff64[5];
    uint64_t buffersSize  = buff64[6];

    auto pack = DataLib::CDataBuffersPack::Create();
    pack->setADCBits(adcBits);
    pack->setOSCRate(oscRate);

    *_id = packId;
    *_allBuffersSize = buffersSize;
    return pack;
}

auto createEndPack(uint64_t _id) -> AsioBufferNolder{
    AsioBufferNolder bh;    
    bh.headerLen = 0;
    bh.dataPtr = nullptr;
    bh.dataLen = 0;
    bh.buffPackOwner = nullptr;

    try{
        uint64_t buffer_lenght = sizeof(int8_t) * 16; // ID of pack (16 byte)
        uint64_t packId = _id;
        buffer_lenght += sizeof(uint64_t) * 2;
        // auto buff = std::shared_ptr<uint8_t[]>(new uint8_t[buffer_lenght]);
        //memcpy_neon(buff.get() ,net_lib::ID_PACK_END,16);
        memcpy_neon(bh.header ,net_lib::ID_PACK_END,16);
        uint64_t* buff64 = reinterpret_cast<uint64_t*>(bh.header);
        buff64[2] = buffer_lenght;
        buff64[3] = packId;        
        bh.headerLen = buffer_lenght;
        return bh;
    } catch (const std::bad_alloc& e) {
        return bh;
    }
}


auto net_lib::extractEndPack(uint8_t* _buffer,size_t _length,uint64_t *_id) -> bool{
    if (_length < 20){ // ID + buff_size attribute
        return  false;
    }

    for(size_t i = 0 ;i < _length && i < 16; i++){
        if (net_lib::ID_PACK_END[i] != _buffer[i]){
            return false;
        }
    }

    uint64_t* buff64    = reinterpret_cast<uint64_t*>(_buffer);
    uint64_t  buff_size = buff64[2];
    if (buff_size > _length){
        return false;
    }
    *_id = buff64[3];
    return true;
}

auto createBufferPack(uint64_t _id,DataLib::EDataBuffersPackChannel channel,DataLib::CDataBuffer::Ptr buffer,size_t split) -> net_list_bh{
    net_list_bh list;

    try{
        uint64_t packOrder = 0;
        auto lenght = buffer->getBufferLenght();
        for(size_t bufferOffset = 0; bufferOffset < lenght; bufferOffset += split){

            AsioBufferNolder bh;            
            bh.headerLen = 0;
            bh.dataPtr = nullptr;
            bh.dataLen = 0;
            bh.buffPackOwner = nullptr;

            auto calcCopyLen = bufferOffset + split > lenght ? lenght - bufferOffset : split;

            uint64_t packId    = _id;
            uint64_t ch_type   = (uint64_t)channel;
            uint64_t adc_mode  = (uint64_t)buffer->getADCMode();
            uint64_t buffSize  = calcCopyLen;
            uint64_t bitBySample64 = buffer->getBitBySample();
            uint64_t lostFPGA = buffer->getLostSamples(DataLib::FPGA);
            uint64_t lostINTERNAL = buffer->getLostSamples(DataLib::RP_INTERNAL_BUFFER);

            uint64_t prefix_size = sizeof(int8_t) * 16 + sizeof(uint64_t) * 9;
            uint64_t buffer_lenght = prefix_size + calcCopyLen;

            // auto buff = std::shared_ptr<uint8_t[]>(new uint8_t[prefix_size]);
            // memcpy_neon(buff.get() ,net_lib::ID_BUFFER,16);
            memcpy_neon(bh.header,net_lib::ID_BUFFER,16);
            
            uint64_t* buff64 = reinterpret_cast<uint64_t*>(bh.header);
            buff64[2] = buffer_lenght;
            buff64[3] = packId;
            buff64[4] = packOrder++;
            buff64[5] = ch_type;
            buff64[6] = adc_mode;
            buff64[7] = bitBySample64;
            buff64[8] = buffSize;
            buff64[9] = lostFPGA;
            buff64[10] = lostINTERNAL;
            
            bh.headerLen = prefix_size;
            bh.dataPtr = buffer->getBuffer().get() + bufferOffset;
            bh.dataLen = calcCopyLen;
            bh.buffPackOwner = buffer;

//            memcpy_neon(buff.get() + prefix_size, buffer->getBuffer().get(), calcCopyLen);
            list.push_back(bh);
        }
    } catch (const std::bad_alloc& e) {
    }
    return list;
}

auto createEmptyBufferPack(uint64_t _id,DataLib::EDataBuffersPackChannel channel,DataLib::CDataBuffer::Ptr buffer) -> net_list_bh{

    AsioBufferNolder bh;
    bh.headerLen = 0;
    bh.dataPtr = nullptr;
    bh.dataLen = 0;
    bh.buffPackOwner = nullptr;

    net_list_bh list;

    uint64_t packOrder = 0;
    uint64_t packId    = _id;
    uint64_t ch_type   = (uint64_t)channel;
    uint64_t adc_mode  = (uint64_t)buffer->getADCMode();
    uint64_t buffSize  = 0;
    uint64_t bitBySample64 = buffer->getBitBySample();
    uint64_t lostFPGA = buffer->getLostSamples(DataLib::FPGA);
    uint64_t lostINTERNAL = buffer->getLostSamples(DataLib::RP_INTERNAL_BUFFER);

    uint64_t prefix_size = sizeof(int8_t) * 16 + sizeof(uint64_t) * 9;
    uint64_t buffer_lenght = prefix_size;

    // auto buff = std::shared_ptr<uint8_t[]>(new uint8_t[buffer_lenght]);
    // memcpy_neon(buff.get() ,net_lib::ID_BUFFER,16);
    memcpy_neon(bh.header ,net_lib::ID_BUFFER,16);
    uint64_t* buff64 = reinterpret_cast<uint64_t*>(bh.header);
    buff64[2] = buffer_lenght;
    buff64[3] = packId;
    buff64[4] = packOrder;
    buff64[5] = ch_type;
    buff64[6] = adc_mode;
    buff64[7] = bitBySample64;
    buff64[8] = buffSize;
    buff64[9] = lostFPGA;
    buff64[10] = lostINTERNAL;

    // bh.header = buff;
    bh.headerLen = buffer_lenght;
    bh.dataPtr = nullptr;
    bh.dataLen = 0;
    bh.buffPackOwner = nullptr;

    list.push_back(bh);

    return list;
}

auto net_lib::extractBufferPack(uint8_t* _buffer,size_t _length,uint64_t *_id,uint64_t *_packOrder,DataLib::EDataBuffersPackChannel *_channel) -> DataLib::CDataBuffer::Ptr{
    if (_length < 20){ // ID + buff_size attribute
        return  nullptr;
    }

    for(size_t i = 0 ;i < _length && i < 16; i++){
        if (net_lib::ID_BUFFER[i] != _buffer[i]){
            return nullptr;
        }
    }

    uint64_t* buff64    = reinterpret_cast<uint64_t*>(_buffer);
    uint64_t  buff_size = buff64[2];
    if (buff_size > _length){
        return nullptr;
    }
    uint64_t packId        = buff64[3];
    uint64_t packOrder     = buff64[4];
    uint64_t ch_type       = buff64[5];
    uint64_t adc_mode      = buff64[6];
    uint64_t bitBySample64 = buff64[7];
    uint64_t dataSize      = buff64[8];
    uint64_t lostFPGA      = buff64[9];
    uint64_t lostINTERNAL  = buff64[10];

    uint64_t prefix_size = sizeof(int8_t) * 16 + sizeof(uint64_t) * 9;

    auto pack = dataSize == 0 ?
                DataLib::CDataBuffer::CreateEmpty(bitBySample64) :
                DataLib::CDataBuffer::Create(_buffer + prefix_size,dataSize,bitBySample64);

    pack->setADCMode((DataLib::CDataBuffer::ADC_MODE)adc_mode);
    pack->setLostSamples(DataLib::FPGA,lostFPGA);
    pack->setLostSamples(DataLib::RP_INTERNAL_BUFFER,lostINTERNAL);

    *_id = packId;
    *_packOrder = packOrder;
    *_channel = (DataLib::EDataBuffersPackChannel)ch_type;

    return pack;
}

auto net_lib::buildPack(uint64_t _id,DataLib::CDataBuffersPack::Ptr pack,size_t split_size) -> net_list_bh{

    net_list_bh list;
    auto begin = createBeginPack(_id,pack);
    auto end = createEndPack(_id);
    list.push_front(begin);

    for(auto i = (int)DataLib::EDataBuffersPackChannel::CH1; i <= (int)DataLib::EDataBuffersPackChannel::CH4 ;i++){
        auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
        if (buff){
            if (buff->getBufferLenght()){
                auto buff_list = createBufferPack(_id,(DataLib::EDataBuffersPackChannel)i,buff,split_size);
                for(auto &v:buff_list){
                    list.push_back(v);
                }
            }else{
                auto buff_list = createEmptyBufferPack(_id,(DataLib::EDataBuffersPackChannel)i,buff);
                for(auto &v:buff_list){
                    list.push_back(v);
                }
            }
        }
    }
    list.push_back(end);
    return list;
}

