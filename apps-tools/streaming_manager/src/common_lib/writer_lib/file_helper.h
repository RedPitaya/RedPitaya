#ifndef WRITER_LIB_FILEHELPER_H
#define WRITER_LIB_FILEHELPER_H

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "data_lib/buffers_pack.h"
#include "net_lib/asio_common.h"
#include "file_chunk.h"
#include "tdms_lib/segment_builder.h"
#include "bin_lib/bin_file.h"

#define USING_FREE_SPACE 1024 * 1024 * 30  // Left free on disk 30 Mb

struct SBuffPass {
    net_lib::net_buffer buffer;
    uint64_t bufferLen;
    uint64_t samplesCount;
    uint8_t bitsBySample;
    uint32_t adcSpeed;
};

enum FH_CSVMode { FH_CSV_NONE = 0, FH_CSV_ADD_TIME_COL_FOR_BLOCK = 0x1, FH_CSV_ADD_TIME_COL = 0x2, FH_CSV_ADD_TIME_COL_NS = 0x4, FH_CSV_ADD_INDEX = 0x8 };

auto getTotalSystemMemory() -> uint64_t;
auto availableSpace(std::string dst, uint64_t* availableSize) -> int;
auto getFreeSpaceDisk(std::string _filePath) -> uint64_t;


// Adds the buffers to a TDMS segment by reference (no copy): the column "time" when there is one,
// then "ch1".."ch4" of the group "Group"
auto addTDMSSamples(TDMS::SegmentBuilder& segment, std::map<DataLib::EDataBuffersPackChannel, SBuffPass>& new_buffs, std::shared_ptr<std::vector<int64_t>> time) -> void;
// The chunk of a finished segment; it keeps the segment, and so the buffers it references, until written
auto buildTDMSChunk(std::shared_ptr<TDMS::SegmentBuilder> segment) -> FileChunk::Ptr;
// Both for a segment of just the buffers
auto buildTDMSChunk(std::map<DataLib::EDataBuffersPackChannel, SBuffPass> new_buffs, std::shared_ptr<std::vector<int64_t>> time) -> FileChunk::Ptr;
// What a BIN file being written keeps between its packs (bin_lib, the format in bin_format.h)
struct BinFileState {
    bool started = false;                // the file header is written
    std::array<int, 4> index{-1, -1, -1, -1};  // CH1..CH4 -> the channel in the file
    bin_lib::Timeline line;
};

// The blocks of a pack, the samples copied once out of the DMA buffers; the first chunk of a
// file starts with the file header made from its pack. _samples: what to take of each channel.
auto buildBINChunk(DataLib::CDataBuffersPackDMA::Ptr buff_pack, std::map<DataLib::EDataBuffersPackChannel, uint32_t> _samples, BinFileState& state) -> FileChunk::Ptr;

auto dirNameOf(const std::string& fname) -> std::string;

#endif
