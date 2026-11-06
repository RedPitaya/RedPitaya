#ifndef BIN_LIB_BIN_FILE_H
#define BIN_LIB_BIN_FILE_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "bin_format.h"

namespace bin_lib {

struct ChannelInfo {
    uint8_t id = 1;
    SampleType type = SampleType::INT16;
    uint16_t flags = 0;
    uint8_t adcBits = 0;
};

struct FileInfo {
    Source source = Source::ADC;
    uint16_t flags = 0;
    uint64_t clock = 0;
    uint32_t decimation = 1;
    int64_t startTime = 0;
    std::vector<ChannelInfo> channels;
};

struct BlockInfo {
    uint8_t channel = 0;  // the index in FileInfo::channels
    uint16_t flags = 0;
    uint32_t triggerCount = 0;
    uint64_t samples = 0;
    uint64_t lost = 0;
    uint64_t wait = 0;
    uint64_t position = 0;
    uint32_t dataSize = 0;  // read: bytes of the data
};

// ---- writing: the bytes go to the file as they are built

// The file header with its channel table
auto fileHeader(const FileInfo& info) -> std::vector<uint8_t>;
// Bytes a block of dataSize bytes takes in the file
auto blockSize(uint32_t dataSize) -> uint64_t;
// Appends a block: its header, the data (one copy) and the padding
auto appendBlock(std::vector<uint8_t>& out, const BlockInfo& block, const void* data, uint32_t dataSize) -> void;

// The time line of the channels of a recording: the position of the next block of each
class Timeline {
   public:
    // the position of a block of `samples` after `lost` + `wait` gap samples; moves past it
    auto next(uint8_t channel, uint64_t lost, uint64_t wait, uint64_t samples) -> uint64_t;
    auto reset() -> void;

   private:
    std::vector<uint64_t> m_end;
};

// ---- reading

class FileReader {
   public:
    // false with error() when the file is not a BIN stream file of a known version
    auto open(const std::string& path) -> bool;
    auto close() -> void;
    auto error() const -> const std::string&;
    auto info() const -> const FileInfo&;

    // The blocks one after the other from the first; false at the end. The data is read into
    // `data` (resized).
    auto rewind() -> void;
    auto nextBlock(BlockInfo& block, std::vector<uint8_t>& data) -> bool;

    // Samples of a channel of a fixed sample size (not RLE) by their position. Without gaps the
    // samples of the blocks follow each other; with gaps the lost and waited samples stand on the
    // time line too, as zero bytes, so that the recording plays in its own time.
    auto samples(uint8_t channel, bool gaps) const -> uint64_t;
    // Up to n samples from position into dst; how many (fewer only at the end)
    auto read(uint8_t channel, uint64_t position, uint64_t n, void* dst, bool gaps) -> uint64_t;

   private:
    struct Piece {
        uint64_t gap;       // samples before the data (lost + wait)
        uint64_t samples;   // of the data
        uint64_t offset;    // of the data in the file
        uint64_t start[2];  // the first sample of the gap on the line without and with gaps
    };

    auto index() -> void;

    std::ifstream m_file;
    std::string m_error;
    FileInfo m_info;
    uint64_t m_firstBlock = 0;
    uint64_t m_next = 0;  // nextBlock(): the file offset of the next block
    uint64_t m_end = 0;   // the size of the file
    std::vector<std::vector<Piece>> m_pieces;  // per channel, of fixed size channels
};

}  // namespace bin_lib

#endif
