#ifndef BIN_LIB_BIN_FORMAT_H
#define BIN_LIB_BIN_FORMAT_H

#include <cstdint>

// The BIN stream file, version 1: what a stream records and what is played back from it.
// Little-endian; every structure and every block is 8 byte aligned.
//
//   FileHeader                      64 bytes
//   ChannelEntry x channels         32 bytes each
//   blocks to the end of the file:  BlockHeader (48 bytes) + data padded to 8 bytes
//
// The blocks of the channels of one buffer follow each other; a block holds one buffer of one
// channel. A recording cut short ends with an incomplete block: it is not read. The time line of
// a channel counts samples: a block starts at `position`, right after the samples lost (FPGA
// overflow) and waited (trigger wait) before it.
namespace bin_lib {

constexpr char FILE_MAGIC[8] = {'R', 'P', 'S', 'T', 'R', 'E', 'A', 'M'};
constexpr uint32_t BLOCK_MAGIC = 0x4B425052;  // "RPBK"
constexpr uint16_t FORMAT_VERSION = 1;

// What the file holds
enum class Source : uint8_t {
    ADC = 1,
    DAC = 2,       // a signal made for the DAC
    GPIO_IN = 3,   // the GPIO input of a board
    GPIO_OUT = 4,  // the words for the GPIO output
};

enum class SampleType : uint8_t {
    INT8 = 1,
    INT16 = 2,
    FLOAT32 = 3,
    // a 32 bit word of the GPIO input {N count, N lines, P count, P lines}; with RLE a lane's
    // level holds count + 1 samples
    GPIO_IN_WORD = 4,
    // a 32 bit word of the GPIO output {P lines, N lines, unused, count}; with RLE the lines hold
    // count + 1 samples
    GPIO_OUT_WORD = 5,
};

// FileHeader::flags
constexpr uint16_t FILE_FLAG_TRIGGER_MODE = 0x0001;  // the recording waited for triggers
constexpr uint16_t FILE_FLAG_WAIT_COUNTER = 0x0002;  // the FPGA counted the wait apart from the loss

// ChannelEntry::flags
constexpr uint16_t CHANNEL_FLAG_RLE = 0x0001;  // the words carry run lengths

struct __attribute__((packed)) FileHeader {
    char magic[8];
    uint16_t version;
    uint16_t headerSize;  // with the channel table: where the first block starts
    uint8_t source;       // Source
    uint8_t channels;
    uint16_t flags;
    uint64_t clock;       // Hz before the decimation
    uint32_t decimation;  // the rate of the samples is clock / decimation
    uint32_t reserved0;
    int64_t startTime;    // UTC, ns since the epoch; 0 - unknown
    uint8_t reserved1[24];
};

struct __attribute__((packed)) ChannelEntry {
    uint8_t id;          // ADC 1..4, DAC 1..2, GPIO 1
    uint8_t sampleType;  // SampleType
    uint16_t flags;
    uint8_t adcBits;     // of the converter (14, 16, 12, ...), 0 - not an ADC
    uint8_t reserved[27];
};

struct __attribute__((packed)) BlockHeader {
    uint32_t magic;  // BLOCK_MAGIC
    uint8_t channel;  // the index in the channel table
    uint8_t reserved0;
    uint16_t flags;
    uint32_t dataSize;      // bytes of data, without the padding
    uint32_t triggerCount;  // the FPGA trigger counter read after the buffer, informative
    uint64_t samples;       // samples of the block; with RLE those its words stand for
    uint64_t lost;          // samples lost right before the block
    uint64_t wait;          // samples waited for a trigger right before the block
    uint64_t position;      // the first sample of the block on the time line of the channel
};

static_assert(sizeof(FileHeader) == 64, "the file header is 64 bytes");
static_assert(sizeof(ChannelEntry) == 32, "a channel entry is 32 bytes");
static_assert(sizeof(BlockHeader) == 48, "a block header is 48 bytes");

// bytes of a sample, 4 for a GPIO word; 0 for an unknown type
constexpr auto sampleBytes(SampleType type) -> uint32_t {
    switch (type) {
        case SampleType::INT8:
            return 1;
        case SampleType::INT16:
            return 2;
        case SampleType::FLOAT32:
        case SampleType::GPIO_IN_WORD:
        case SampleType::GPIO_OUT_WORD:
            return 4;
    }
    return 0;
}

constexpr auto padded(uint64_t bytes) -> uint64_t {
    return (bytes + 7) & ~uint64_t(7);
}

}  // namespace bin_lib

#endif
