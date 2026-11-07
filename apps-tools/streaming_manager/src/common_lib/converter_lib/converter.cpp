#include <fstream>
#include <iostream>

#include "converter.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <vector>

#include "bin_lib/bin_file.h"
#include "bin_lib/gpio_words.h"
#include "data_lib/network_header.h"
#include "writer_lib/gpio_file_writer.h"
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

// A recording of the GPIO: one channel of GPIO words
auto isGpio(const bin_lib::FileInfo& info) -> bool {
    return info.channels.size() == 1 &&
           (info.channels[0].type == bin_lib::SampleType::GPIO_IN_WORD || info.channels[0].type == bin_lib::SampleType::GPIO_OUT_WORD);
}

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
    if (isGpio(info))
        return gpioToText(reader, _file_name, CStreamSettings::GPIOFormat::CSV, _prefix);
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

bool CConverter::convertToWAV(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix) {
    std::lock_guard lock(m_mtx);
    m_stopWriteWAV = false;
    return toStreamFile(_file_name, start_seg, end_seg, _prefix, CStreamSettings::DataFormat::WAV, m_stopWriteWAV);
}

bool CConverter::convertToTDMS(std::string _file_name, std::string _prefix) {
    return convertToTDMS(_file_name, -2, -2, _prefix);
}

bool CConverter::convertToTDMS(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix) {
    std::lock_guard lock(m_mtx);
    m_stopWriteTDMS = false;
    return toStreamFile(_file_name, start_seg, end_seg, _prefix, CStreamSettings::DataFormat::TDMS, m_stopWriteTDMS);
}

bool CConverter::convertToVCD(std::string _file_name, std::string _prefix) {
    std::lock_guard lock(m_mtx);
    if (_prefix != "")
        _prefix = "[" + _prefix + "] ";
    bin_lib::FileReader reader;
    if (!reader.open(_file_name)) {
        aprintf(stderr, "%s Error open file %s: %s\n", _prefix.c_str(), _file_name.c_str(), reader.error().c_str());
        return false;
    }
    if (!isGpio(reader.info())) {
        aprintf(stderr, "%s VCD is made of GPIO recordings only\n", _prefix.c_str());
        return false;
    }
    return gpioToText(reader, _file_name, CStreamSettings::GPIOFormat::VCD, _prefix);
}

// The packs of an ADC recording through CStreamingFile, as the board records WAV and TDMS
auto CConverter::toStreamFile(const std::string& file, int32_t start_seg, int32_t end_seg, std::string prefix, CStreamSettings::DataFormat format,
                              std::atomic_bool& stop) -> bool {
    if (prefix != "")
        prefix = "[" + prefix + "] ";
    const char* name = format.value == CStreamSettings::DataFormat::WAV ? "WAV" : "TDMS";
    aprintf(stdout, "%s Started converting to %s\n", prefix.c_str(), name);
    bin_lib::FileReader reader;
    if (!reader.open(file)) {
        aprintf(stderr, "%s Error open file %s: %s\n", prefix.c_str(), file.c_str(), reader.error().c_str());
        return false;
    }
    const auto& info = reader.info();
    if (isGpio(info)) {
        aprintf(stderr, "%s A GPIO recording goes to CSV or VCD, not to %s\n", prefix.c_str(), name);
        return false;
    }
    const std::string out = file.substr(0, file.size() - 4) + (format.value == CStreamSettings::DataFormat::WAV ? ".wav" : ".tdms");
    std::string dir = ".";
    auto writer = CStreamingFile::create(format, dir, 0, false, false);
    writer->stopNotify.connect([&](CStreamingFile::EStopReason r) {
        if (r == CStreamingFile::OUT_SPACE) {
            aprintf(stdout, "%s Disk is full\n", prefix.c_str());
            stop = true;
        }
    });
    writer->run("", out);

    const uint64_t length = std::filesystem::file_size(file);
    const uint32_t decimation = info.decimation ? info.decimation : 1;
    start_seg = std::max(start_seg, 1);
    Pack pack;
    bin_lib::BlockInfo held;
    std::vector<uint8_t> heldData;
    bool haveHeld = false;
    uint64_t done = 0;
    bool ret = true;
    for (int32_t segment = 1; nextPack(reader, pack, held, heldData, haveHeld); segment++) {
        if (stop) {
            aprintf(stdout, "%s Abort writing to %s file\n", prefix.c_str(), name);
            ret = false;
            break;
        }
        if (end_seg != -2 && segment > end_seg)
            break;
        if (segment < start_seg)
            continue;
        auto p = DataLib::CDataBuffersPackDMA::Create();
        for (size_t k = 0; k < pack.blocks.size(); k++) {
            const auto& b = pack.blocks[k];
            const auto& ch = info.channels[b.channel];
            if (ch.id < 1 || ch.id > 4)
                continue;
            const uint8_t bits = static_cast<uint8_t>(bin_lib::sampleBytes(ch.type) * 8);
            auto* bytes = new uint8_t[pack.data[k].size()];  // the buffer takes it over
            std::memcpy(bytes, pack.data[k].data(), pack.data[k].size());
            auto buff = DataLib::CDataBufferDMA::Create(bytes, pack.data[k].size(), bits);
            buff->setADCBaseRate(info.clock / decimation);
            buff->setADCBaseBits(ch.adcBits ? ch.adcBits : bits);
            buff->setADCClock(static_cast<uint32_t>(info.clock));
            buff->setDecimation(info.decimation);
            buff->setLostSamples(DataLib::FPGA, b.lost);
            buff->setLostSamples(DataLib::WAIT, b.wait);
            buff->setTriggerCount(b.triggerCount);
            buff->setADCFlags(static_cast<uint8_t>(((info.flags & bin_lib::FILE_FLAG_TRIGGER_MODE) ? DataLib::ADC_FLAG_TRIGGER_MODE : 0) |
                                                   ((info.flags & bin_lib::FILE_FLAG_WAIT_COUNTER) ? DataLib::ADC_FLAG_WAIT_COUNTER : 0)));
            const double period = info.clock ? 1e9 * decimation / static_cast<double>(info.clock) : 0;
            buff->setTimeCapture(info.startTime ? info.startTime + static_cast<int64_t>(period * static_cast<double>(b.position)) : 0);
            p->addBuffer(static_cast<DataLib::EDataBuffersPackChannel>(ch.id - 1), buff);
            done += bin_lib::blockSize(b.dataSize);
        }
        p->setOSCRate(info.clock / decimation);
        writer->passBuffers(p);
        if (length)
            aprintf(stdout, "\r%s PROGRESS: %d %%", prefix.c_str(), static_cast<int>(std::min<uint64_t>(100, done * 100 / length)));
    }
    writer->stopAndFlush();
    aprintf(stdout, "\n%s Ended converting\n", prefix.c_str());
    return ret;
}

// A GPIO recording (input words, or output words seen as input words of both lanes) into
// CSV or VCD by the writer the board records them with
auto CConverter::gpioToText(bin_lib::FileReader& reader, const std::string& file, CStreamSettings::GPIOFormat format, const std::string& prefix) -> bool {
    const auto& info = reader.info();
    const auto& ch = info.channels[0];
    GpioRecording rec;
    rec.format = format;
    rec.rle = (ch.flags & bin_lib::CHANNEL_FLAG_RLE) != 0;
    rec.clock = info.clock;
    rec.decimation = info.decimation ? info.decimation : 1;
    rec.triggerMode = info.flags & bin_lib::FILE_FLAG_TRIGGER_MODE;
    rec.waitCounter = info.flags & bin_lib::FILE_FLAG_WAIT_COUNTER;
    rec.startTime = info.startTime;
    const std::string out = file.substr(0, file.size() - 3) + (format.value == CStreamSettings::GPIOFormat::VCD ? "vcd" : "csv");
    aprintf(stdout, "%s %s\n", prefix.c_str(), out.c_str());
    CGpioFileWriter writer;
    if (!writer.open(out, rec)) {
        aprintf(stderr, "%s Error open file %s\n", prefix.c_str(), out.c_str());
        return false;
    }
    bin_lib::BlockInfo b;
    std::vector<uint8_t> data;
    std::vector<uint32_t> words;
    bool ret = true;
    while (reader.nextBlock(b, data)) {
        const auto* w = reinterpret_cast<const uint32_t*>(data.data());
        const size_t count = data.size() / 4;
        if (ch.type == bin_lib::SampleType::GPIO_OUT_WORD) {
            words.resize(count);
            for (size_t i = 0; i < count; i++)
                words[i] = bin_lib::inWord(bin_lib::outP(w[i]), bin_lib::outCount(w[i]), bin_lib::outN(w[i]), bin_lib::outCount(w[i]));
            w = words.data();
        }
        const auto r = writer.write(w, count, b.lost, b.wait, b.triggerCount);
        if (r != CGpioFileWriter::Result::OK) {
            ret = r == CGpioFileWriter::Result::LIMIT;
            break;
        }
    }
    writer.close();
    aprintf(stdout, "%s Ended converting\n", prefix.c_str());
    return ret;
}
