#ifndef DATA_LIB_NETWORK_HEAER_H
#define DATA_LIB_NETWORK_HEAER_H

#include <stdint.h>

#include "buffer.h"

namespace DataLib {

extern const uint8_t ID_PACK_ADC[];
extern const uint8_t ID_PACK_DAC[];
extern const uint8_t ID_PACK_GPIO[];

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
    uint32_t adcClock = {0};  // ADC clock before the decimation, Hz: the exact rate is adcClock / decimation
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

// GPIOHeader::flags
constexpr uint8_t GPIO_FLAG_RLE = 0x01;           // a word carries its run length: count + 1 samples
constexpr uint8_t GPIO_FLAG_TRIGGER_MODE = 0x02;  // the input waits for a trigger
constexpr uint8_t GPIO_FLAG_WAIT_COUNTER = 0x04;  // the FPGA counts the wait apart from the loss

// One buffer of 32 bit words: the input {N count, N lines, P count, P lines} from the board, the
// output {P lines, N lines, 8 bit unused, count} to the board
struct __attribute__((packed)) GPIOHeader {
    uint64_t lostFPGA = {0};  // input: samples lost right before this buffer (overflow, not the trigger wait)
    uint64_t waitFPGA = {0};  // input: samples skipped right before this buffer while waiting for a trigger
    uint32_t dataSize = {0};  // bytes of words in the buffer: the last output buffer may be filled in part
    uint32_t decimation = {0};
    uint32_t gpioClock = {0};  // Hz before the decimation
    uint32_t triggerCount = {0};  // input: FPGA trigger counter read after the buffer, informative
    uint16_t triggerBlocks = {0};
    uint8_t flags = {0};
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
// The whole header of a GPIO buffer: the pack ID, the size and `gpio`
auto setHeaderGPIO(CDataBufferDMA::Ptr buffer, uint64_t id, const GPIOHeader& gpio) -> void;
// false when the buffer holds no GPIO header
auto getHeaderGPIO(CDataBufferDMA::Ptr buffer, GPIOHeader& gpio, uint64_t* id = nullptr) -> bool;
auto sizeHeader() -> uint16_t;
auto printADCHeader(uint8_t* data) -> void;
auto printDACHeader(uint8_t* data) -> void;
}  // namespace DataLib
#endif
