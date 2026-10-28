#ifndef DATA_LIB_NETWORK_HEAER_H
#define DATA_LIB_NETWORK_HEAER_H

#include <stdint.h>

#include "buffer.h"

namespace DataLib {

extern const uint8_t ID_PACK_ADC[];
extern const uint8_t ID_PACK_DAC[];

// ADCHeader::flags
constexpr uint8_t ADC_FLAG_TRIGGER_MODE = 0x01;  // the acquisition waits for a trigger
constexpr uint8_t ADC_FLAG_WAIT_COUNTER = 0x02;  // the FPGA counts the wait apart from the loss

struct __attribute__((packed)) ADCHeader {
    uint64_t lostFPGA = {0};  // samples lost right before this buffer (overflow, not the trigger wait)
    uint64_t sizeOfAllChannels = {0};
    uint32_t baseRateOSC = {0};
    uint8_t baseOSCBits = {0};
    uint8_t channel = {0};
    uint8_t bitBySample = {0};
    uint8_t adcMode = {0};
    int64_t timeCapture = {0};
    uint64_t waitFPGA = {0};  // samples skipped right before this buffer while waiting for a trigger
    uint32_t decimation = {0};
    uint32_t triggerCount = {0};  // FPGA trigger counter read after the buffer, informative
    uint16_t triggerBlocks = {0};
    uint8_t flags = {0};
    ADCHeader(){};
};

struct __attribute__((packed)) DACHeader {
    uint8_t channels = {0};
    uint8_t channel = {0};
    uint32_t channelSize = {0};
    uint64_t sizeOfAllChannels = {0};
    bool onePackMode = false;
    bool infMode = false;
    int64_t repeatCount = {0};
    uint8_t bits = {16};
    DACHeader(){};
};

struct __attribute__((packed)) GPIOHeader {
    uint8_t channels;
    GPIOHeader(){};
};

struct __attribute__((packed)) NetworkPackHeader {
    uint8_t ID_PACK[16];
    uint64_t bufferSize{0};  // Data size + header size
    uint64_t packId{0};

    union {
        ADCHeader adc;
        DACHeader dac;
        GPIOHeader gpio;
    } __attribute__((packed));

    NetworkPackHeader(){};
};

static_assert(sizeof(NetworkPackHeader) <= 128, "the header block of a buffer is 128 bytes");

auto initHeaderADC(CDataBufferDMA::Ptr buffer, uint64_t oscRate, uint64_t adcBits, uint64_t buffersSize, uint8_t channel) -> void;
auto initHeaderDAC(CDataBufferDMA::Ptr buffer, uint64_t buffersSize, uint8_t channels) -> void;
auto setHeaderADC(CDataBufferDMA::Ptr buffer, uint64_t _id) -> void;
auto setHeaderDAC(CDataBufferDMA::Ptr buffer, uint8_t channel, uint32_t channelSize, bool onePackMode, bool infMode, int64_t repeatCount, uint8_t bits) -> void;
auto sizeHeader() -> uint16_t;
auto printADCHeader(uint8_t* data) -> void;
auto printDACHeader(uint8_t* data) -> void;
}  // namespace DataLib
#endif
