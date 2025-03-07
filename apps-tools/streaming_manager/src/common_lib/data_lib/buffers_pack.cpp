#include "buffers_pack.h"
#include <inttypes.h>
#include "logger_lib/file_logger.h"
#include "network_header.h"

using namespace DataLib;

auto CDataBuffersPackDMA::Create() -> CDataBuffersPackDMA::Ptr {
    return std::make_shared<CDataBuffersPackDMA>();
}

CDataBuffersPackDMA::CDataBuffersPackDMA() : m_buffers() {}

CDataBuffersPackDMA::~CDataBuffersPackDMA() {}

auto CDataBuffersPackDMA::addBuffer(EDataBuffersPackChannel channel, DataLib::CDataBufferDMA::Ptr buffer) -> void {
    m_buffers[channel] = buffer;
}

auto CDataBuffersPackDMA::getBuffer(EDataBuffersPackChannel channel) const -> DataLib::CDataBufferDMA::Ptr {
    if (m_buffers.count(channel)) {
        return m_buffers.at(channel);
    }
    return nullptr;
}

auto CDataBuffersPackDMA::getBufferDataAddress(EDataBuffersPackChannel channel) -> uint32_t {
    if (m_buffers.count(channel)) {
        return m_buffers.at(channel)->getDataAddress();
    }
    return 0;
}

auto CDataBuffersPackDMA::isChannelPresent(EDataBuffersPackChannel channel) -> bool {
    return m_buffers.find(channel) != m_buffers.end();
}

auto CDataBuffersPackDMA::setOSCRate(uint64_t rate) -> void {
    for (const auto& kv : m_buffers) {
        kv.second->setADCBaseRate(rate);
    }
}

auto CDataBuffersPackDMA::setADCBits(uint8_t bits) -> void {
    for (const auto& kv : m_buffers) {
        kv.second->setADCBaseBits(bits);
    }
}

auto CDataBuffersPackDMA::checkDataBuffersEqual() -> bool {
    size_t size = 0;
    uint8_t bits = 0;
    for (const auto& kv : m_buffers) {
        auto buff_size = kv.second->getDataLenght();
        auto buff_bit = kv.second->getBitBySample();
        if (buff_size) {
            if (size != 0) {
                if (size != buff_size) {
                    return false;
                }
            } else {
                size = buff_size;
            }
        }
        if (buff_bit) {
            if (bits != 0) {
                if (bits != buff_bit) {
                    return false;
                }
            } else {
                bits = buff_bit;
            }
        }
    }
    return true;
}

auto CDataBuffersPackDMA::getDataBuffersLenght() -> size_t {
    size_t size = 0;
    for (const auto& kv : m_buffers) {
        if (kv.second->getDataLenght()) {
            if (size < kv.second->getDataLenght()) {
                if (size > 0) {
                    WARNING("The buffers are not the same length. The program may not work correctly.")
                }
                size = kv.second->getDataLenght();
            }
        }
    }
    return size;
}

auto CDataBuffersPackDMA::getLenghtBuffers() -> uint64_t {
    uint64_t size = 0;
    for (const auto& kv : m_buffers) {
        size += kv.second->getBufferFullLenght();
    }
    return size;
}

auto CDataBuffersPackDMA::getLenghtDataBuffers() -> uint64_t {
    uint64_t size = 0;
    for (const auto& kv : m_buffers) {
        size += kv.second->getDataLenght();
    }
    return size;
}

auto CDataBuffersPackDMA::getLostAll() -> uint64_t {
    uint64_t size = 0;
    for (const auto& kv : m_buffers) {
        size += kv.second->getLostSamplesInBytesLenght();
    }
    return size;
}

auto CDataBuffersPackDMA::getBuffersSamples() -> size_t {
    size_t size = 0;
    for (const auto& kv : m_buffers) {
        if (kv.second->getSamplesCount()) {
            if (size < kv.second->getSamplesCount()) {
                size = kv.second->getSamplesCount();
            }
        }
    }
    return size;
}

auto CDataBuffersPackDMA::debugPackADC() -> void {
    printf("Channels %d\n", (uint32_t)m_buffers.size());

    for (auto& item : m_buffers) {
        printf("\tChannel %d\n", item.first);
        printf("\t\tHeader %d\n", (uint32_t)item.second->getHeaderLenght());
        printADCHeader((uint8_t*)item.second->getMappedMemory());
        printf("\t\tData %d\n", (uint32_t)item.second->getDataLenght());
        printf("\t\t\tMemory: ");
        for (size_t i = 0; i < 100 && i < item.second->getBufferFullLenght(); i++) {
            printf("%X,", ((uint8_t*)item.second->getMappedMemory())[i]);
        }
        printf("\n\t\t\tData: ");
        for (size_t i = 0; i < 200 && i < item.second->getDataLenght(); i++) {
            printf("%X,", ((uint8_t*)item.second->getMappedDataMemory())[i]);
        }
        printf("\n");
    }
}

auto CDataBuffersPackDMA::debugPackDAC() -> void {
    printf("Channels %d\n", (uint32_t)m_buffers.size());

    for (auto& item : m_buffers) {
        printf("\tChannel %d\n", item.first);
        printf("\t\tHeader %d\n", (uint32_t)item.second->getHeaderLenght());
        printDACHeader((uint8_t*)item.second->getMappedMemory());
        printf("\t\tData %d\n", (uint32_t)item.second->getDataLenght());
        printf("\t\t\tMemory: ");
        for (size_t i = 0; i < 100 && i < item.second->getBufferFullLenght(); i++) {
            printf("%X,", ((uint8_t*)item.second->getMappedMemory())[i]);
        }
        printf("\n\t\t\tData: ");
        int dev = item.second->getDACBits() / 8;
        for (size_t i = 0; i < 200 && i < item.second->getDataLenght() / dev; i++) {
            if (dev == 2)
                printf("%X,", ((uint16_t*)item.second->getMappedDataMemory())[i]);
            if (dev == 1)
                printf("%X,", ((uint8_t*)item.second->getMappedDataMemory())[i]);
        }
        printf("\n");
    }
}

auto CDataBuffersPackDMA::verifyPack() -> void {
    for (auto& item : m_buffers) {
        auto header = reinterpret_cast<NetworkPackHeader*>(item.second->getMappedMemory());
        if (header->bufferSize != item.second->getBufferFullLenght()) {
            FATAL("The buffer is not the correct size. Header size %" PRIu64 ". Pack lenght %u", header->bufferSize,
                  (uint32_t)item.second->getBufferFullLenght())
        }
    }
}

auto CDataBuffersPackDMA::getInfoFromHeaderADC() -> void {
    for (auto& item : m_buffers) {
        auto header = reinterpret_cast<NetworkPackHeader*>(item.second->getMappedMemory());
        item.second->setADCBaseBits(header->adc.baseOSCBits);
        item.second->setADCBaseRate(header->adc.baseRateOSC);
        item.second->setLostSamples(EDataLost::FPGA, header->adc.lostFPGA);
        item.second->setBitBySample(header->adc.bitBySample);
        item.second->setADCPackId(header->packId);
    }
}

auto CDataBuffersPackDMA::getInfoFromHeaderDAC() -> void {
    for (auto& item : m_buffers) {
        auto header = reinterpret_cast<NetworkPackHeader*>(item.second->getMappedMemory());
        item.second->setDACOnePackMode(header->dac.onePackMode);
        item.second->setDACRepeatCount(header->dac.repeatCount);
        item.second->setDACChannelSize(header->dac.channelSize);
        item.second->setDACInfMode(header->dac.infMode);
        item.second->setDACBits(header->dac.bits);
    }
}

auto CDataBuffersPackDMA::resetWriteSizeAll() -> void {
    for (const auto& kv : m_buffers) {
        kv.second->resetWriteSize();
    }
}

auto CDataBuffersPackDMA::getNextDMABufferForWrite() -> CDataBufferDMA::Ptr {
    for (auto i = (int)DataLib::EDataBuffersPackChannel::CH1; i <= (int)DataLib::EDataBuffersPackChannel::CH4; i++) {
        auto buff = getBuffer((DataLib::EDataBuffersPackChannel)i);
        if (buff != nullptr) {
            if (buff->getWriteSizeLeft() > 0)
                return buff;
        }
    }
    return nullptr;
}

auto CDataBuffersPackDMA::isAllDataWrite() -> bool {
    bool flag = true;
    for (auto i = (int)DataLib::EDataBuffersPackChannel::CH1; i <= (int)DataLib::EDataBuffersPackChannel::CH4; i++) {
        auto buff = getBuffer((DataLib::EDataBuffersPackChannel)i);
        if (buff != nullptr) {
            flag &= buff->getWriteSizeLeft() == 0;
        }
    }
    return flag;
}
