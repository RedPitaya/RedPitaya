#ifndef DATA_LIB_BUFFER_H
#define DATA_LIB_BUFFER_H

#include <stdint.h>
#include <map>
#include <memory>

namespace DataLib {

// FPGA: samples lost before the buffer (overflow). WAIT: samples skipped before the buffer while
// the acquisition waited for a trigger, a pause and not a loss.
enum EDataLost { FPGA = 0, WAIT = 1 };

class CDataBufferDMA final {
   public:
    enum ADC_MODE { ATT_1_1 = 0, ATT_1_20 = 1 };

    using Ptr = std::shared_ptr<DataLib::CDataBufferDMA>;

    static auto Create(uint32_t startAddress, size_t lenght, void* mappedMemory, uint8_t bitsBySample) -> CDataBufferDMA::Ptr;
    static auto Create(uint8_t* buffer, size_t lenght, uint8_t bits) -> CDataBufferDMA::Ptr;
    static auto CreateEmpty(uint8_t bitsBySample) -> CDataBufferDMA::Ptr;

    CDataBufferDMA(uint32_t startDataAddress, size_t lenght, void* mappedMemory, uint8_t bits);
    CDataBufferDMA(uint8_t* buffer, size_t lenght, uint8_t bits);
    CDataBufferDMA(uint8_t bitsBySample);
    ~CDataBufferDMA();

    auto initHeaderAddress(size_t headerSize) -> void;
    auto getBuffer() const -> uint32_t;
    auto getBufferFullLenght() const -> size_t;
    auto getHeaderAddress() const -> uint32_t;
    auto getHeaderLenght() const -> size_t;
    auto getDataAddress() const -> uint32_t;
    auto getDataLenght() const -> size_t;
    auto setBitBySample(uint8_t bits) -> void;
    auto getBitBySample() const -> uint8_t;
    auto getSamplesCount() const -> size_t;
    auto getSamplesWithLost() const -> uint64_t;
    auto getMappedMemory() -> void*;
    auto getMappedDataMemory() -> void*;

    auto setADCMode(ADC_MODE mode) -> void;
    auto getADCMode() -> ADC_MODE;

    auto setADCBaseBits(uint8_t bits) -> void;
    auto getADCBaseBits() -> uint8_t;

    auto setADCPackId(uint64_t id) -> void;
    auto getADCPackId() -> uint64_t;

    auto setADCBaseRate(uint64_t rate) -> void;
    auto getADCBaseRate() -> uint64_t;

    auto setDACOnePackMode(bool enable) -> void;
    auto getDACOnePackMode() -> bool;

    auto setDACRepeatCount(int64_t count) -> void;
    auto getDACRepeatCount() -> int64_t;
    auto decDACRepeatCount() -> void;

    auto setDACInfMode(bool enable) -> void;
    auto getDACInfMode() -> bool;

    auto setDACChannelSize(uint32_t size) -> void;
    auto getDACChannelSize() -> uint32_t;

    auto setDACBits(uint8_t bits) -> void;
    auto getDACBits() -> uint8_t;

    auto setTimeCapture(int64_t time) -> void;
    auto getTimeCapture() -> int64_t;

    auto setLostSamples(EDataLost mode, uint64_t value) -> void;
    auto getLostSamples(EDataLost mode) const -> uint64_t;
    auto getLostSamplesAll() const -> uint64_t;
    auto getLostSamplesInBytesLenght() -> uint64_t;
    auto getWaitSamples() const -> uint64_t;
    auto getSamplesWithGaps() const -> uint64_t;  // samples, lost and wait: the time the buffer covers

    auto setDecimation(uint32_t decimation) -> void;
    auto getDecimation() const -> uint32_t;
    auto setTriggerCount(uint32_t count) -> void;
    auto getTriggerCount() const -> uint32_t;
    auto setTriggerBlocks(uint16_t blocks) -> void;
    auto getTriggerBlocks() const -> uint16_t;
    auto setADCFlags(uint8_t flags) -> void;  // ADC_FLAG_* of network_header.h
    auto getADCFlags() const -> uint8_t;

    auto reset() -> void;

    auto resetWriteSize() -> void;
    auto getWriteSize() -> uint32_t;
    auto getWriteSizeLeft() -> uint32_t;
    auto addWriteSize(uint32_t size) -> void;

   private:
    CDataBufferDMA(const CDataBufferDMA&) = delete;
    CDataBufferDMA(CDataBufferDMA&&) = delete;
    CDataBufferDMA& operator=(const CDataBufferDMA&) = delete;
    CDataBufferDMA& operator=(const CDataBufferDMA&&) = delete;

    uint32_t m_bufferAddress = 0;
    uint32_t m_headerAddress = 0;
    uint32_t m_dataAddress = 0;
    size_t m_lenght = 0;
    size_t m_headerLenght = 0;
    uint8_t m_bitBySample = 0;  // Resolution 8/16/32 bits
    void* m_mappedMemory = nullptr;
    void* m_mappedDataMemory = nullptr;
    bool m_needDelete = false;
    ADC_MODE m_adcMode = ATT_1_1;
    std::map<EDataLost, uint64_t> m_lost;
    uint32_t m_writeSize = 0;
    uint8_t m_baseADCBits = 0;
    uint64_t m_baseADCRate = 0;
    uint64_t m_packId = 0;

    bool m_onePackModeDAC = false;
    bool m_infModeDAC = false;
    int64_t m_repeatCountDAC = 0;
    uint32_t m_channelSizeDAC = 0;
    uint8_t m_dacBits = 0;
    int64_t m_timeCapture = 0;
    uint32_t m_decimation = 0;
    uint32_t m_triggerCount = 0;
    uint16_t m_triggerBlocks = 0;
    uint8_t m_adcFlags = 0;
};

}  // namespace DataLib

#endif
