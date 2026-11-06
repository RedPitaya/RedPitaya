#include <fstream>
#include <iostream>

#include "converter.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <vector>

#include "bin_lib/bin_file.h"
#include "data_lib/buffer.h"
#include "logger_lib/file_logger.h"
#include "streaming_lib/streaming_file.h"

using namespace converter_lib;
using namespace streaming_lib;

auto CConverter::create() -> CConverter::Ptr {
    return std::make_shared<CConverter>();
}

CConverter::CConverter() {
    m_stopWriteCSV = false;
}

CConverter::~CConverter() {
    stopWriteToCSV();
    stopWriteToWAV();
    stopWriteToTDMS();
    std::lock_guard lock(m_mtx);  // wait while file converting
}

auto CConverter::stopWriteToCSV() -> void {
    m_stopWriteCSV = true;
}

void CConverter::stopWriteToWAV() {
    m_stopWriteWAV = true;
}

void CConverter::stopWriteToTDMS() {
    m_stopWriteTDMS = true;
}

bool CConverter::convertToCSV(std::string _file_name, std::string _prefix, FH_CSVMode mode) {
    return convertToCSV(_file_name, -2, -2, _prefix, mode);
}

namespace {

// The BIN stream file of bin_lib (version 1) replaced the old segment format; the conversions to
// WAV and TDMS come back with the new tools (CSV is there: rpsa_client records CSV through it).
auto notYet(const std::string& file, const char* to) -> bool {
    ERROR_LOG("%s: converting a BIN stream file to %s is not supported yet (BIN format version 1)", file.c_str(), to)
    return false;
}

}  // namespace

namespace {

// The blocks of one buffer pack: the writer puts the channels of a pack in their order, a block
// of a channel not after the one before starts the next pack
struct Pack {
    std::vector<bin_lib::BlockInfo> blocks;
    std::vector<std::vector<uint8_t>> data;
};

auto nextPack(bin_lib::FileReader& reader, Pack& pack, bin_lib::BlockInfo& held, std::vector<uint8_t>& heldData, bool& haveHeld) -> bool {
    pack.blocks.clear();
    pack.data.clear();
    if (haveHeld) {
        pack.blocks.push_back(held);
        pack.data.push_back(heldData);
        haveHeld = false;
    }
    bin_lib::BlockInfo b;
    std::vector<uint8_t> d;
    while (reader.nextBlock(b, d)) {
        if (!pack.blocks.empty() && b.channel <= pack.blocks.back().channel) {
            held = b;
            heldData = d;
            haveHeld = true;
            break;
        }
        pack.blocks.push_back(b);
        pack.data.push_back(d);
    }
    return !pack.blocks.empty();
}

auto printSample(std::ostream& out, const uint8_t* data, uint64_t i, bin_lib::SampleType type) -> void {
    switch (type) {
        case bin_lib::SampleType::INT8:
            out << static_cast<int>(reinterpret_cast<const int8_t*>(data)[i]);
            break;
        case bin_lib::SampleType::INT16:
            out << reinterpret_cast<const int16_t*>(data)[i];
            break;
        case bin_lib::SampleType::FLOAT32:
            out << reinterpret_cast<const float*>(data)[i];
            break;
        default:
            out << reinterpret_cast<const uint32_t*>(data)[i];
            break;
    }
}

// The rows of a pack: per sample a row with a column per channel of the file; the samples lost
// and waited before a block of a channel are "-" after its samples
auto packToCSV(std::ostream& out, const bin_lib::FileInfo& info, const Pack& pack, uint64_t& samplePos, FH_CSVMode mode) -> void {
    const size_t channels = info.channels.size();
    std::vector<const bin_lib::BlockInfo*> block(channels, nullptr);
    std::vector<const uint8_t*> data(channels, nullptr);
    uint64_t rows = 0;
    for (size_t k = 0; k < pack.blocks.size(); k++) {
        const auto& b = pack.blocks[k];
        block[b.channel] = &b;
        data[b.channel] = pack.data[k].data();
        rows = std::max(rows, b.samples + b.lost + b.wait);
    }
    const double period = info.clock ? 1e9 * info.decimation / static_cast<double>(info.clock) : 0;
    const uint64_t position = pack.blocks.front().position;
    for (uint64_t i = 0; i < rows; i++) {
        samplePos++;
        if (mode & FH_CSV_ADD_INDEX)
            out << samplePos << ",";
        if (info.startTime != 0) {
            if ((mode & FH_CSV_ADD_TIME_COL) || (mode & FH_CSV_ADD_TIME_COL_NS) || ((mode & FH_CSV_ADD_TIME_COL_FOR_BLOCK) && i == 0)) {
                const int64_t ns = info.startTime + static_cast<int64_t>(period * static_cast<double>(position + i));
                if (mode & FH_CSV_ADD_TIME_COL_NS) {
                    out << ns << ",";
                } else {
                    std::time_t t = static_cast<std::time_t>(ns / 1000000000LL);
                    char buf[100];
                    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::gmtime(&t));
                    out << buf << "." << std::setfill('0') << std::setw(9) << (ns % 1000000000LL) << ",";
                }
            } else if (mode & FH_CSV_ADD_TIME_COL_FOR_BLOCK) {
                out << ",";
            }
        }
        bool comma = false;
        for (size_t c = 0; c < channels; c++) {
            if (!block[c])
                continue;
            if (comma)
                out << ",";
            comma = true;
            if (i < block[c]->samples)
                printSample(out, data[c], i, info.channels[c].type);
            else
                out << "-";
        }
        out << "\n";
    }
}

}  // namespace

bool CConverter::convertToCSV(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix, FH_CSVMode mode) {
    std::lock_guard lock(m_mtx);
    m_stopWriteCSV = false;
    if (_prefix != "")
        _prefix = "[" + _prefix + "] ";
    aprintf(stdout, "%s Started converting to CSV\n", _prefix.c_str());
    const std::string csv_file = _file_name.substr(0, _file_name.size() - 3) + "csv";
    aprintf(stdout, "%s %s\n", _prefix.c_str(), csv_file.c_str());

    bin_lib::FileReader reader;
    if (!reader.open(_file_name)) {
        aprintf(stderr, "%s Error open file %s: %s\n", _prefix.c_str(), _file_name.c_str(), reader.error().c_str());
        return false;
    }
    const auto& info = reader.info();
    for (const auto& ch : info.channels) {
        if (ch.flags & bin_lib::CHANNEL_FLAG_RLE) {
            aprintf(stderr, "%s The file has RLE words: not converted to CSV yet\n", _prefix.c_str());
            return false;
        }
    }
    std::ofstream out(csv_file, std::ios::trunc);
    if (!out) {
        aprintf(stderr, "%s Error open file %s\n", _prefix.c_str(), csv_file.c_str());
        return false;
    }
    const uint64_t length = std::filesystem::file_size(_file_name);
    start_seg = std::max(start_seg, 1);
    Pack pack;
    bin_lib::BlockInfo held;
    std::vector<uint8_t> heldData;
    bool haveHeld = false;
    uint64_t samplePos = 0;
    uint64_t done = 0;
    bool ret = true;
    for (int32_t segment = 1; nextPack(reader, pack, held, heldData, haveHeld); segment++) {
        if (m_stopWriteCSV) {
            aprintf(stdout, "%s Abort writing to CSV file\n", _prefix.c_str());
            ret = false;
            break;
        }
        if (getFreeSpaceDisk(csv_file) <= USING_FREE_SPACE) {
            aprintf(stdout, "%s Disk is full\n", _prefix.c_str());
            ret = false;
            break;
        }
        if (end_seg != -2 && segment > end_seg)
            break;
        // the samples before the first pack written still count in the index
        if (segment < start_seg) {
            uint64_t rows = 0;
            for (const auto& b : pack.blocks)
                rows = std::max(rows, b.samples + b.lost + b.wait);
            samplePos += rows;
            continue;
        }
        packToCSV(out, info, pack, samplePos, mode);
        for (const auto& b : pack.blocks)
            done += bin_lib::blockSize(b.dataSize);
        if (length)
            aprintf(stdout, "\r%s PROGRESS: %d %%", _prefix.c_str(), static_cast<int>(std::min<uint64_t>(100, done * 100 / length)));
        if (!out) {
            aprintf(stdout, "\n%s Error write to CSV file\n", _prefix.c_str());
            ret = false;
            break;
        }
    }
    aprintf(stdout, "\n%s Ended converting\n", _prefix.c_str());
    return ret;
}

bool CConverter::convertToWAV(std::string _file_name, std::string _prefix) {
    return convertToWAV(_file_name, -2, -2, _prefix);
}

bool CConverter::convertToWAV(std::string _file_name, int32_t, int32_t, std::string) {
    std::lock_guard lock(m_mtx);
    return notYet(_file_name, "WAV");
}

bool CConverter::convertToTDMS(std::string _file_name, std::string _prefix) {
    return convertToTDMS(_file_name, -2, -2, _prefix);
}

bool CConverter::convertToTDMS(std::string _file_name, int32_t, int32_t, std::string) {
    std::lock_guard lock(m_mtx);
    return notYet(_file_name, "TDMS");
}
