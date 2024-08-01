#ifndef DATA_LIB_BUFFER_PACK_H
#define DATA_LIB_BUFFER_PACK_H

#include <stdint.h>
#include <memory>
#include <map>
#include <mutex>
#include "buffer.h"

namespace DataLib {

enum EDataBuffersPackChannel{
    CH1 = 0,
    CH2 = 1,
    CH3 = 2,
    CH4 = 3
};

class CDataBuffersPack final{

public:

    using Ptr = std::shared_ptr<DataLib::CDataBuffersPack>;

    static auto Create() -> CDataBuffersPack::Ptr;

    CDataBuffersPack();
    ~CDataBuffersPack();

    auto addBuffer(EDataBuffersPackChannel channel,DataLib::CDataBuffer::Ptr buffer) -> void;
    auto getBuffer(EDataBuffersPackChannel channel) const -> DataLib::CDataBuffer::Ptr;

    auto setOSCRate(uint64_t rate) -> void;
    auto getOSCRate() -> uint64_t;
    auto setADCBits(uint8_t bits) -> void;
    auto getADCBits() -> uint8_t;

    auto checkBuffersEqual() -> bool;
    auto getBuffersLenght() -> size_t;
    auto getBuffersSamples() -> size_t;
    auto getLenghtAllBuffers() -> uint64_t;
    auto getLostAllBuffers() -> uint64_t;
    auto isChannelPresent(EDataBuffersPackChannel channel) -> bool;

private:

    CDataBuffersPack(const CDataBuffersPack &) = delete;
    CDataBuffersPack(CDataBuffersPack &&) = delete;
    CDataBuffersPack& operator=(const CDataBuffersPack&) =delete;
    CDataBuffersPack& operator=(const CDataBuffersPack&&) =delete;

    std::map<EDataBuffersPackChannel,CDataBuffer::Ptr> m_buffers;
    uint64_t m_oscRate; // Decimation
    uint8_t  m_adc_bits;
};

}

#endif
