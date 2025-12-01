#include "buffer.h"
#include "logger_lib/file_logger.h"

using namespace DataLib;

auto CDataBufferDMA::Create(uint32_t startAddress, size_t lenght, void* mappedMemory, uint8_t bitsBySample) -> CDataBufferDMA::Ptr {
    return std::make_shared<CDataBufferDMA>(startAddress, lenght, mappedMemory, bitsBySample);
}

auto CDataBufferDMA::Create(uint8_t* buffer, size_t lenght, uint8_t bitsBySample) -> CDataBufferDMA::Ptr {
    return std::make_shared<CDataBufferDMA>(buffer, lenght, bitsBySample);
}

auto CDataBufferDMA::CreateEmpty(uint8_t bitsBySample) -> CDataBufferDMA::Ptr {
    return std::make_shared<CDataBufferDMA>(bitsBySample);
}

CDataBufferDMA::CDataBufferDMA(uint8_t bitsBySample)
    : m_bufferAddress(0),
      m_headerAddress(0),
      m_dataAddress(0),
      m_lenght(0),
      m_headerLenght(0),
      m_bitBySample(bitsBySample),
      m_mappedMemory(0),
      m_mappedDataMemory(0),
      m_needDelete(false),
      m_adcMode(ATT_1_1),
      m_lost(),
      m_packId(0),
      m_onePackModeDAC(false),
      m_infModeDAC(false),
      m_repeatCountDAC(0),
      m_channelSizeDAC(0) {
    setLostSamples(EDataLost::FPGA, 0);
}

CDataBufferDMA::CDataBufferDMA(uint32_t startAddress, size_t lenght, void* mappedMemory, uint8_t bits)
    : m_bufferAddress(startAddress),
      m_headerAddress(0),
      m_dataAddress(startAddress),
      m_lenght(lenght),
      m_headerLenght(0),
      m_bitBySample(bits),
      m_mappedMemory(mappedMemory),
      m_mappedDataMemory(mappedMemory),
      m_needDelete(false),
      m_adcMode(ATT_1_1),
      m_lost(),
      m_packId(0),
      m_onePackModeDAC(false),
      m_infModeDAC(false),
      m_repeatCountDAC(0),
      m_channelSizeDAC(0) {
    setLostSamples(EDataLost::FPGA, 0);
}

CDataBufferDMA::CDataBufferDMA(uint8_t* buffer, size_t lenght, uint8_t bits)
    : m_bufferAddress(0),
      m_headerAddress(0),
      m_dataAddress(0),
      m_lenght(lenght),
      m_headerLenght(0),
      m_bitBySample(bits),
      m_mappedMemory(buffer),
      m_mappedDataMemory(buffer),
      m_needDelete(true),
      m_adcMode(ATT_1_1),
      m_lost(),
      m_packId(0),
      m_onePackModeDAC(false),
      m_infModeDAC(false),
      m_repeatCountDAC(0),
      m_channelSizeDAC(0) {
    setLostSamples(EDataLost::FPGA, 0);
}

CDataBufferDMA::~CDataBufferDMA() {
    if (m_needDelete) {
        delete[] reinterpret_cast<uint8_t*>(m_mappedMemory);
        m_mappedMemory = NULL;
        m_mappedDataMemory = NULL;
    }
}

auto CDataBufferDMA::reset() -> void {
    setLostSamples(EDataLost::FPGA, 0);
}

auto CDataBufferDMA::getBuffer() const -> uint32_t {
    return m_bufferAddress;
}

auto CDataBufferDMA::getBufferFullLenght() const -> size_t {
    return m_lenght;
}

auto CDataBufferDMA::getHeaderAddress() const -> uint32_t {
    return m_headerAddress;
}

auto CDataBufferDMA::getHeaderLenght() const -> size_t {
    return m_headerLenght;
}

auto CDataBufferDMA::getDataAddress() const -> uint32_t {
    return m_dataAddress;
}

auto CDataBufferDMA::getDataLenght() const -> size_t {
    return m_lenght - getHeaderLenght();
}

auto CDataBufferDMA::setBitBySample(uint8_t bits) -> void {
    m_bitBySample = bits;
}

auto CDataBufferDMA::getBitBySample() const -> uint8_t {
    return m_bitBySample;
}

auto CDataBufferDMA::getSamplesCount() const -> size_t {
    return getDataLenght() / (m_bitBySample / 8);
}

auto CDataBufferDMA::getSamplesWithLost() const -> uint64_t {
    return getSamplesCount() + getLostSamplesAll();
}

auto CDataBufferDMA::setADCMode(CDataBufferDMA::ADC_MODE mode) -> void {
    m_adcMode = mode;
}

auto CDataBufferDMA::getADCMode() -> CDataBufferDMA::ADC_MODE {
    return m_adcMode;
}

auto CDataBufferDMA::setADCPackId(uint64_t id) -> void {
    m_packId = id;
}

auto CDataBufferDMA::getADCPackId() -> uint64_t {
    return m_packId;
}

auto CDataBufferDMA::setLostSamples(EDataLost mode, uint64_t value) -> void {
    m_lost[mode] = value;
}

auto CDataBufferDMA::getLostSamples(EDataLost mode) const -> uint64_t {
    return m_lost.at(mode);
}

auto CDataBufferDMA::getLostSamplesAll() const -> uint64_t {
    return getLostSamples(DataLib::FPGA);
}

auto CDataBufferDMA::getLostSamplesInBytesLenght() -> uint64_t {
    auto lost = getLostSamples(DataLib::FPGA);
    return lost * (getBitBySample() / 8);
}

auto CDataBufferDMA::initHeaderAddress(size_t headerSize) -> void {
    m_headerAddress = m_bufferAddress;
    m_dataAddress = m_bufferAddress + headerSize;
    m_headerLenght = headerSize;
    m_mappedDataMemory = reinterpret_cast<uint8_t*>(m_mappedMemory) + headerSize;
}

auto CDataBufferDMA::getMappedMemory() -> void* {
    return m_mappedMemory;
}

auto CDataBufferDMA::getMappedDataMemory() -> void* {
    return m_mappedDataMemory;
}

auto CDataBufferDMA::resetWriteSize() -> void {
    m_writeSize = 0;
}

auto CDataBufferDMA::getWriteSize() -> uint32_t {
    return m_writeSize;
}

auto CDataBufferDMA::getWriteSizeLeft() -> uint32_t {
    return getBufferFullLenght() - m_writeSize;
}

auto CDataBufferDMA::addWriteSize(uint32_t size) -> void {
    m_writeSize += size;
    if (m_writeSize > getBufferFullLenght()) {
        FATAL("Write size greater than allowed. Writed: %d. Required: %d", m_writeSize, (uint32_t)getBufferFullLenght())
    }
}

auto CDataBufferDMA::setADCBaseBits(uint8_t bits) -> void {
    m_baseADCBits = bits;
}

auto CDataBufferDMA::getADCBaseBits() -> uint8_t {
    return m_baseADCBits;
}

auto CDataBufferDMA::setADCBaseRate(uint64_t rate) -> void {
    m_baseADCRate = rate;
}

auto CDataBufferDMA::getADCBaseRate() -> uint64_t {
    return m_baseADCRate;
}

auto CDataBufferDMA::setDACOnePackMode(bool enable) -> void {
    m_onePackModeDAC = enable;
}

auto CDataBufferDMA::getDACOnePackMode() -> bool {
    return m_onePackModeDAC;
}

auto CDataBufferDMA::setDACInfMode(bool enable) -> void {
    m_infModeDAC = enable;
}

auto CDataBufferDMA::getDACInfMode() -> bool {
    return m_infModeDAC;
}

auto CDataBufferDMA::setDACRepeatCount(int64_t count) -> void {
    m_repeatCountDAC = count;
}

auto CDataBufferDMA::getDACRepeatCount() -> int64_t {
    return m_repeatCountDAC;
}

auto CDataBufferDMA::decDACRepeatCount() -> void {
    if (m_repeatCountDAC > 0) {
        m_repeatCountDAC--;
    }
}

auto CDataBufferDMA::setDACChannelSize(uint32_t size) -> void {
    m_channelSizeDAC = size;
}

auto CDataBufferDMA::getDACChannelSize() -> uint32_t {
    return m_channelSizeDAC;
}

auto CDataBufferDMA::setDACBits(uint8_t bits) -> void {
    m_dacBits = bits;
}

auto CDataBufferDMA::getDACBits() -> uint8_t {
    return m_dacBits;
}