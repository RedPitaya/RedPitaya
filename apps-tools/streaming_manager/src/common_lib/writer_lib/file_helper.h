#ifndef WRITER_LIB_FILEHELPER_H
#define WRITER_LIB_FILEHELPER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "data_lib/buffers_pack.h"
#include "net_lib/asio_common.h"
#include "w_binary.h"

#define USING_FREE_SPACE 1024 * 1024 * 30  // Left free on disk 30 Mb

struct SBuffPass {
    net_lib::net_buffer buffer;
    uint64_t bufferLen;
    uint64_t samplesCount;
    uint8_t bitsBySample;
    uint32_t adcSpeed;
};

struct SBinData {
    uint8_t* ch[4] = {nullptr, nullptr, nullptr, nullptr};
    uint64_t ch_size[4] = {0, 0, 0, 0};
    uint64_t ch_samples[4] = {0, 0, 0, 0};
    uint64_t ch_lost[4] = {0, 0, 0, 0};
    uint64_t ch_bits[4] = {0, 0, 0, 0};
    int64_t ch_timeCapture[4] = {0, 0, 0, 0};
    uint64_t adcRate = 0;

    ~SBinData() {
        delete[] ch[0];
        delete[] ch[1];
        delete[] ch[2];
        delete[] ch[3];
    }
};

enum FH_CSVMode { FH_CSV_NONE = 0, FH_CSV_ADD_TIME_COL_FOR_BLOCK = 0x1, FH_CSV_ADD_TIME_COL = 0x2, FH_CSV_ADD_TIME_COL_NS = 0x4, FH_CSV_ADD_INDEX = 0x8 };

auto getTotalSystemMemory() -> uint64_t;
auto availableSpace(std::string dst, uint64_t* availableSize) -> int;
auto getFreeSpaceDisk(std::string _filePath) -> uint64_t;

auto readBinInfo(std::iostream* buffer) -> CBinInfo;
auto readCSV(std::iostream* buffer, int64_t* _position, int* _channels, uint64_t* samplePos, bool skipData = false, FH_CSVMode mode = FH_CSVMode::FH_CSV_NONE) -> std::iostream*;
auto readBinData(std::iostream* buffer, int64_t* _position) -> SBinData*;

auto buildTDMSStream(std::map<DataLib::EDataBuffersPackChannel, SBuffPass> new_buffs, std::shared_ptr<std::vector<int64_t>> time) -> std::iostream*;
auto buildBINStream(DataLib::CDataBuffersPackDMA::Ptr buff_pack, std::map<DataLib::EDataBuffersPackChannel, uint32_t> _samples) -> std::iostream*;

auto dirNameOf(const std::string& fname) -> std::string;

#endif
