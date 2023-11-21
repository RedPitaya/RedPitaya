#ifndef WRITER_LIB_FILEHELPER_H
#define WRITER_LIB_FILEHELPER_H

#include <string>
#include <map>

#include "w_binary.h"
#include "data_lib/buffers_pack.h"
#include "net_lib/asio_common.h"

#define USING_FREE_SPACE 1024 * 1024 * 30 // Left free on disk 30 Mb

struct SBuffPass{
    net_lib::net_buffer buffer;
    size_t bufferLen;
    size_t samplesCount;
    uint8_t bitsBySample;
    uint32_t adcSpeed;
};

auto getTotalSystemMemory() -> uint64_t;
auto availableSpace(std::string dst, uint64_t* availableSize) -> int;
auto getFreeSpaceDisk(std::string _filePath) ->  uint64_t;

auto readBinInfo(std::iostream *buffer) -> CBinInfo;
auto readCSV(std::iostream *buffer,int64_t *_position,int *_channels,uint64_t *samplePos,bool skipData = false) -> std::iostream *;

auto buildTDMSStream(std::map<DataLib::EDataBuffersPackChannel,SBuffPass> new_buffs) -> std::iostream *;
auto buildBINStream (DataLib::CDataBuffersPack::Ptr buff_pack, std::map<DataLib::EDataBuffersPackChannel,uint32_t> _samples) -> std::iostream *;

auto dirNameOf(const std::string& fname) -> std::string;

#endif
