#ifndef DATA_LIB_BUFFER_H
#define DATA_LIB_BUFFER_H

#include <stdint.h>
#include <memory>
#include <map>

namespace DataLib {

enum EDataLost{
    FPGA = 0,
    RP_INTERNAL_BUFFER = 1
};

class CDataBuffer final{

public:

    enum ADC_MODE{
        ATT_1_1  = 0,
        ATT_1_20 = 1
    };

    using Ptr = std::shared_ptr<DataLib::CDataBuffer>;

    static auto Create16Bit(uint8_t *buffer,size_t lenght) -> CDataBuffer::Ptr;
    static auto Create8BitFrom16Bit(uint8_t *buffer,size_t lenght) -> CDataBuffer::Ptr;
    static auto Create(uint8_t *buffer,size_t lenght,uint8_t bitsBySample) -> CDataBuffer::Ptr;
    static auto Create(std::shared_ptr<uint8_t[]> buffer,size_t lenght,uint8_t bitsBySample) -> CDataBuffer::Ptr;
    static auto CreateEmpty(uint8_t bitsBySample) -> CDataBuffer::Ptr;

    CDataBuffer(std::shared_ptr<uint8_t[]> buffer,size_t lenght,uint8_t bits);
    CDataBuffer(uint8_t *buffer,size_t lenght,uint8_t bits,bool simpleCopy);
    CDataBuffer(uint8_t bitsBySample);
    ~CDataBuffer();

    auto recalcBufferLenght() -> void;
    auto getBuffer() const -> std::shared_ptr<uint8_t[]>;
    auto getBufferLenght() const -> size_t;
    auto getBitBySample() const -> uint8_t;
    auto getSamplesCount() const -> size_t;
    auto getSamplesWithLost() const -> uint64_t;


    auto setADCMode(ADC_MODE mode) -> void;
    auto getADCMode() -> ADC_MODE;

    auto setLostSamples(EDataLost mode,uint64_t value) -> void;
    auto getLostSamples(EDataLost mode) const -> uint64_t;
    auto getLostSamplesAll() const -> uint64_t;
    auto getLostSamplesInBytesLenght() -> uint64_t;

    auto reset() -> void;

private:

    CDataBuffer(const CDataBuffer &) = delete;
    CDataBuffer(CDataBuffer &&) = delete;
    CDataBuffer& operator=(const CDataBuffer&) =delete;
    CDataBuffer& operator=(const CDataBuffer&&) =delete;

    std::shared_ptr<uint8_t[]> m_data;
    size_t   m_lenght;
    uint8_t  m_bitBySample;     // Resolution 8/16/32 bits
    size_t   m_samplesCount;
    ADC_MODE m_adcMode;
    std::map<EDataLost,uint64_t> m_lost;
};

}

#endif
