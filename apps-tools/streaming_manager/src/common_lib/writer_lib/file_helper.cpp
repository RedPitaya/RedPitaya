#ifndef _WIN32
#include <cstdio>
#include <sys/statvfs.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <limits>
#include <sstream>

#include "file_helper.h"
#include "logger_lib/file_logger.h"
#include "data_lib/neon_asm.h"
#include "tdms_lib/tdms.h"
#include "bin_lib/bin_file.h"
#include "data_lib/network_header.h"


auto getTotalSystemMemory() -> uint64_t {
#ifndef _WIN32
    uint64_t pages = sysconf(_SC_PHYS_PAGES);
    uint64_t page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#else
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#endif
}

auto availableSpace(std::string dst, uint64_t* availableSize) -> int {
    std::error_code ec;
    const std::filesystem::space_info si = std::filesystem::space(dst, ec);
    if (!ec) {
        *availableSize = si.available;
        return 0;
    } else {
        std::filesystem::path path = dst;
        if (path.has_parent_path()) {
            return availableSpace(path.parent_path().generic_string(), availableSize);
        } else {
            WARNING("Error in AvailableSpace(): %s\n", ec.message().c_str());
            *availableSize = 0;
        }
    }
    return -1;
}

auto getFreeSpaceDisk(std::string _filePath) -> uint64_t {
    uint64_t m_freeSize = 0;
    if (availableSpace(_filePath, &m_freeSize) == 0) {
        if (m_freeSize < USING_FREE_SPACE) {
            m_freeSize = 0;
        } else {
            m_freeSize = m_freeSize - USING_FREE_SPACE;
        }
    } else {
        m_freeSize = 0;
    }
    return m_freeSize;
}

auto dirNameOf(const std::string& fname) -> std::string {
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos) ? "." : fname.substr(0, pos);
}

auto addTDMSSamples(TDMS::SegmentBuilder& segment, std::map<DataLib::EDataBuffersPackChannel, SBuffPass>& new_buffs, std::shared_ptr<std::vector<int64_t>> time) -> void {
    if (time != nullptr && time->size() != 0) {
        segment.addSamples("Group", "time", time);
    }
    const char* names[4] = {"ch1", "ch2", "ch3", "ch4"};
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        auto it = new_buffs.find((DataLib::EDataBuffersPackChannel)i);
        if (it == new_buffs.end() || !it->second.bufferLen || !it->second.buffer)
            continue;
        const auto& b = it->second;
        auto type = TDMS::TDMSType::Integer8;
        if (b.bitsBySample == 16)
            type = TDMS::TDMSType::Integer16;
        if (b.bitsBySample == 32)
            type = TDMS::TDMSType::SingleFloat;
        segment.addSamples("Group", names[i], type, std::shared_ptr<const std::uint8_t[]>(b.buffer), b.samplesCount);
    }
}

auto buildTDMSChunk(std::shared_ptr<TDMS::SegmentBuilder> segment) -> FileChunk::Ptr {
    auto chunk = std::make_unique<FileChunk>();
    for (const auto& part : segment->parts()) {
        chunk->add(segment, part.data, part.size);
    }
    return chunk;
}

auto buildTDMSChunk(std::map<DataLib::EDataBuffersPackChannel, SBuffPass> new_buffs, std::shared_ptr<std::vector<int64_t>> time) -> FileChunk::Ptr {
    auto segment = std::make_shared<TDMS::SegmentBuilder>();
    addTDMSSamples(*segment, new_buffs, time);
    return buildTDMSChunk(segment);
}

// The channel of a buffer pack in the file: CH1..CH4 in their order, as the first pack has them
namespace {

auto sampleType(uint8_t bits) -> bin_lib::SampleType {
    if (bits == 8)
        return bin_lib::SampleType::INT8;
    if (bits == 32)
        return bin_lib::SampleType::FLOAT32;
    return bin_lib::SampleType::INT16;
}

auto adcFileInfo(DataLib::CDataBuffersPackDMA::Ptr pack, BinFileState& state) -> bin_lib::FileInfo {
    bin_lib::FileInfo info;
    info.source = bin_lib::Source::ADC;
    state.index.fill(-1);
    for (int i = 0; i < 4; i++) {
        auto ch = pack->getBuffer(static_cast<DataLib::EDataBuffersPackChannel>(i));
        if (!ch)
            continue;
        if (info.channels.empty()) {
            const uint32_t dec = ch->getDecimation();
            info.decimation = dec ? dec : 1;
            // the exact clock when the buffer has it, else the rate of the samples
            info.clock = ch->getADCClock() ? ch->getADCClock() : (dec ? ch->getADCBaseRate() * dec : ch->getADCBaseRate());
            info.startTime = ch->getTimeCapture();
            if (ch->getADCFlags() & DataLib::ADC_FLAG_TRIGGER_MODE)
                info.flags |= bin_lib::FILE_FLAG_TRIGGER_MODE;
            if (ch->getADCFlags() & DataLib::ADC_FLAG_WAIT_COUNTER)
                info.flags |= bin_lib::FILE_FLAG_WAIT_COUNTER;
        }
        state.index[i] = static_cast<int>(info.channels.size());
        bin_lib::ChannelInfo c;
        c.id = static_cast<uint8_t>(i + 1);
        c.type = sampleType(ch->getBitBySample());
        c.adcBits = ch->getADCBaseBits();
        info.channels.push_back(c);
    }
    return info;
}

}  // namespace

auto buildBINChunk(DataLib::CDataBuffersPackDMA::Ptr buff_pack, std::map<DataLib::EDataBuffersPackChannel, uint32_t> _samples, BinFileState& state) -> FileChunk::Ptr {
    std::vector<uint8_t> bytes;
    if (!state.started) {
        // the file header with the channels of the first pack
        bytes = bin_lib::fileHeader(adcFileInfo(buff_pack, state));
        state.line.reset();
        state.started = true;
    }
    // One copy out of the DMA buffers, which go back to the ring right after
    for (int i = 0; i < 4; i++) {
        auto ch = buff_pack->getBuffer(static_cast<DataLib::EDataBuffersPackChannel>(i));
        if (!ch || state.index[i] < 0 || _samples[static_cast<DataLib::EDataBuffersPackChannel>(i)] == 0)
            continue;  // past the sample limit: nothing more of this channel
        const uint32_t sampleBytes = ch->getBitBySample() / 8 ? ch->getBitBySample() / 8 : 1;
        uint64_t samples = std::min<uint64_t>(ch->getSamplesCount(), _samples[static_cast<DataLib::EDataBuffersPackChannel>(i)]);
        const uint32_t size = static_cast<uint32_t>(std::min<uint64_t>(samples * sampleBytes, ch->getDataLenght()));
        samples = size / sampleBytes;
        bin_lib::BlockInfo block;
        block.channel = static_cast<uint8_t>(state.index[i]);
        block.triggerCount = ch->getTriggerCount();
        block.samples = samples;
        block.lost = ch->getLostSamples(DataLib::FPGA);
        block.wait = ch->getWaitSamples();
        block.position = state.line.next(block.channel, block.lost, block.wait, samples);
        bin_lib::appendBlock(bytes, block, ch->getMappedDataMemory(), size);
    }
    auto chunk = std::make_unique<FileChunk>();
    chunk->add(std::move(bytes));
    return chunk;
}

auto partFileName(const std::string& base, int part) -> std::string {
    char n[16];
    snprintf(n, sizeof(n), "_%04d", part);
    const size_t slash = base.find_last_of("/\\");
    const size_t dot = base.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return base + n;
    return base.substr(0, dot) + n + base.substr(dot);
}
