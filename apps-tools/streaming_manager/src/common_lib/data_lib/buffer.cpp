#include <iostream>
#include <chrono>
#include "buffer.h"
#include "neon_asm.h"
#include "thread_cout.h"

using namespace DataLib;

auto CDataBuffer::Create16Bit(uint8_t *buffer,size_t lenght) -> CDataBuffer::Ptr{
    return std::make_shared<CDataBuffer>(buffer,lenght,16,false);
}

auto CDataBuffer::Create8BitFrom16Bit(uint8_t *buffer,size_t lenght) -> CDataBuffer::Ptr{
    return std::make_shared<CDataBuffer>(buffer,lenght,8,false);
}

auto CDataBuffer::Create(uint8_t *buffer,size_t lenght,uint8_t bitsBySample) -> CDataBuffer::Ptr{
    return std::make_shared<CDataBuffer>(buffer,lenght,bitsBySample,true);
}

auto CDataBuffer::Create(std::shared_ptr<uint8_t[]> buffer,size_t lenght,uint8_t bitsBySample) -> CDataBuffer::Ptr{
    return std::make_shared<CDataBuffer>(buffer,lenght,bitsBySample);
}

auto CDataBuffer::CreateEmpty(uint8_t bitsBySample) -> CDataBuffer::Ptr{
    return std::make_shared<CDataBuffer>(bitsBySample);
}

CDataBuffer::CDataBuffer(uint8_t bitsBySample):
    m_data(nullptr)
   ,m_lenght(0)
   ,m_bitBySample(bitsBySample)
   ,m_samplesCount(0)
   ,m_adcMode(ATT_1_1)
   ,m_lost()
{
    setLostSamples(EDataLost::FPGA,0);
    setLostSamples(EDataLost::RP_INTERNAL_BUFFER,0);
}

CDataBuffer::CDataBuffer(std::shared_ptr<uint8_t[]> buffer,size_t lenght,uint8_t bits):
    m_data(buffer)
   ,m_lenght(lenght)
   ,m_bitBySample(bits)
   ,m_samplesCount(lenght / (bits/8))
   ,m_adcMode(ATT_1_1)
   ,m_lost()
{
    setLostSamples(EDataLost::FPGA,0);
    setLostSamples(EDataLost::RP_INTERNAL_BUFFER,0);
}

CDataBuffer::CDataBuffer(uint8_t *buffer,size_t lenght,uint8_t bits,bool simpleCopy):
    m_data(nullptr)
   ,m_lenght(0)
   ,m_bitBySample(0)
   ,m_samplesCount(0)
   ,m_adcMode(ATT_1_1)
{
    if (!simpleCopy){
        if (lenght % 2 != 0){
            aprintf(stderr,"[FATAL ERROR] CDataBuffer: %s\n","Buffer must be an even size");
            return;
        }

        if (buffer && lenght > 0 && bits == 16){
            try{
               m_data = std::shared_ptr<uint8_t[]>(new uint8_t[lenght]);
               memcpy_neon(m_data.get(),buffer,lenght);
               m_lenght = lenght;
               m_bitBySample = 16;
               m_samplesCount = lenght / 2;
            }catch(std::exception &e){
                aprintf(stderr,"[ERROR] CDataBuffer: %s\n",e.what());
            }
        }

        if (buffer && lenght > 0 && bits == 8){

            try{
               m_data = std::shared_ptr<uint8_t[]>(new uint8_t[lenght]);
#ifdef ARM_NEON
               memcpy_stride_8bit_neon(m_data.get(),buffer,lenght);
#else
               auto b16 = reinterpret_cast<uint16_t*>(buffer);
               for(auto i = 0u; i < lenght /2 ;i++){
                   m_data[i] = b16[i] >> 8;
               }
#endif
               m_lenght = lenght / 2;
               m_bitBySample = bits;
               m_samplesCount = m_lenght;
            }catch(std::exception &e){
                aprintf(stderr,"[ERROR] CDataBuffer: %s\n",e.what());
            }
        }
    }
    else{
        try{
            if (lenght % (bits / 8) != 0){
                aprintf(stderr,"[FATAL ERROR] CDataBuffer: %s\n","Buffer must be an even size");
                return;
            }
            m_data = std::shared_ptr<uint8_t[]>(new uint8_t[lenght]);
            memcpy_neon(m_data.get(),buffer,lenght);
            m_lenght = lenght;
            m_bitBySample = bits;
            m_samplesCount = lenght / (bits / 8);
        }catch(std::exception &e){
            aprintf(stderr,"[ERROR] CDataBuffer: %s\n",e.what());
        }
    }
    setLostSamples(EDataLost::FPGA,0);
    setLostSamples(EDataLost::RP_INTERNAL_BUFFER,0);
}

CDataBuffer::~CDataBuffer(){
}

auto CDataBuffer::reset() -> void{
    m_lenght = 0;
    m_samplesCount = 0;
    setLostSamples(EDataLost::FPGA,0);
    setLostSamples(EDataLost::RP_INTERNAL_BUFFER,0);
}

auto CDataBuffer::recalcBufferLenght() -> void{
    m_lenght = m_samplesCount * m_bitBySample / 8;
}

auto CDataBuffer::getBuffer() const -> std::shared_ptr<uint8_t[]>{
    return m_data;
}

auto CDataBuffer::getBufferLenght() const -> size_t{
    return m_lenght;
}

auto CDataBuffer::getBitBySample() const -> uint8_t{
    return m_bitBySample;
}

auto CDataBuffer::getSamplesCount() const -> size_t{
    return m_samplesCount;
}

auto CDataBuffer::getSamplesWithLost() const -> uint64_t {
    return getSamplesCount() + getLostSamplesAll();
}

auto CDataBuffer::setADCMode(CDataBuffer::ADC_MODE mode) -> void{
    m_adcMode = mode;
}

auto CDataBuffer::getADCMode() -> CDataBuffer::ADC_MODE{
    return m_adcMode;
}

auto CDataBuffer::setLostSamples(EDataLost mode,uint64_t value) -> void{
    m_lost[mode] = value;
}

auto CDataBuffer::getLostSamples(EDataLost mode) const -> uint64_t{
    return m_lost.at(mode);
}

auto CDataBuffer::getLostSamplesAll() const -> uint64_t{
    return getLostSamples(DataLib::FPGA) + getLostSamples(DataLib::RP_INTERNAL_BUFFER);
}

auto CDataBuffer::getLostSamplesInBytesLenght() -> uint64_t{
    auto lost = getLostSamples(DataLib::FPGA) + getLostSamples(DataLib::RP_INTERNAL_BUFFER);
    return lost * (getBitBySample() / 8);
}
