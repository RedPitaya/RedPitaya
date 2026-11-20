/**
 * The segments of the recording formats and CStreamingFile writing them.
 *
 * BIN is the stream file of bin_lib: the file header with the first chunk, a block per channel
 * and pack, read back by bin_lib::FileReader. TDMS segments reference the converted buffers
 * and are read back by the new FileReader and by the former TDMS::File, which reader_lib still
 * uses for the DAC. WAV keeps its header and interleaving.
 */

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <atomic>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "data_lib/tests/support.h"
#include "streaming_lib/streaming_file.h"
#include "tdms_lib/file.h"
#include "tdms_lib/tdms.h"
#include "wav_lib/wav_writer.h"
#include "bin_lib/bin_file.h"
#include "writer_lib/file_helper.h"

using data_test::Arena;
using data_test::MakeBuffer;

namespace {

constexpr size_t kBlock = 64;  // 32 samples of 16 bit

auto TempDir() -> std::string {
    auto dir = std::filesystem::temp_directory_path() / ("writer_formats_" + std::to_string(::getpid()));
    std::filesystem::create_directories(dir);
    return dir.string();
}

auto Bytes(const FileChunk& c) -> std::string {
    std::ostringstream out(std::ios::binary);
    c.writeTo(out);
    return out.str();
}

auto ReadFile(const std::string& path) -> std::string {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// A pack of two 16 bit channels; channel 2 is the negated channel 1
auto MakePack(Arena& arena, int n, uint64_t lost = 0) -> DataLib::CDataBuffersPackDMA::Ptr {
    auto pack = DataLib::CDataBuffersPackDMA::Create();
    for (int c = 0; c < 2; c++) {
        auto b = MakeBuffer(arena, (n * 2 + c) % 8, 16);
        auto* d = static_cast<int16_t*>(b->getMappedDataMemory());
        for (size_t i = 0; i < b->getSamplesCount(); i++)
            d[i] = static_cast<int16_t>((c ? -1 : 1) * (n * 100 + static_cast<int>(i)));
        b->setADCBaseRate(125000000);
        b->setTimeCapture(1000000 + n);
        b->setLostSamples(DataLib::FPGA, lost);
        pack->addBuffer(c ? DataLib::CH2 : DataLib::CH1, b);
    }
    pack->setOSCRate(125000000);
    return pack;
}

auto Pass(const std::vector<int16_t>& samples, int bits = 16) -> SBuffPass {
    SBuffPass p;
    p.buffer = net_lib::createBuffer(reinterpret_cast<const char*>(samples.data()), samples.size() * 2);
    p.bufferLen = samples.size() * 2;
    p.samplesCount = samples.size();
    p.bitsBySample = bits;
    p.adcSpeed = 125000000;
    return p;
}

}  // namespace

namespace {

auto WriteFile(const std::string& path, const std::string& bytes) -> void {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

}  // namespace

TEST(BinChunk, TheFirstChunkCarriesTheFileHeaderAndTheBlocksTheirTimeline) {
    Arena arena(8, kBlock);
    BinFileState state;
    std::map<DataLib::EDataBuffersPackChannel, uint32_t> all{{DataLib::CH1, 32}, {DataLib::CH2, 32}};
    std::string bytes = Bytes(*buildBINChunk(MakePack(arena, 0), all, state));
    ASSERT_GE(bytes.size(), sizeof(bin_lib::FileHeader));
    EXPECT_EQ(bytes.substr(0, 8), "RPSTREAM");
    const std::string second = Bytes(*buildBINChunk(MakePack(arena, 1, 7), all, state));
    EXPECT_NE(second.substr(0, 8), "RPSTREAM") << "the header once";
    bytes += second;
    const std::string path = TempDir() + "/chunks.bin";
    WriteFile(path, bytes);

    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(path)) << r.error();
    ASSERT_EQ(r.info().channels.size(), 2u);
    EXPECT_EQ(r.info().source, bin_lib::Source::ADC);
    EXPECT_EQ(r.info().channels[0].type, bin_lib::SampleType::INT16);
    EXPECT_EQ(r.info().startTime, 1000000);
    bin_lib::BlockInfo b;
    std::vector<uint8_t> data;
    std::vector<uint64_t> positions;
    while (r.nextBlock(b, data))
        if (b.channel == 0)
            positions.push_back(b.position);
    EXPECT_EQ(positions, (std::vector<uint64_t>{0, 39})) << "32 samples, then 7 lost";
    std::vector<int16_t> ch2(64);
    ASSERT_EQ(r.read(1, 0, 64, ch2.data(), false), 64u);
    EXPECT_EQ(ch2[33], -101);
}

TEST(BinChunk, TheSampleLimitCutsTheBlock) {
    Arena arena(8, kBlock);
    BinFileState state;
    const std::string path = TempDir() + "/limit.bin";
    WriteFile(path, Bytes(*buildBINChunk(MakePack(arena, 3), {{DataLib::CH1, 5}, {DataLib::CH2, 0}}, state)));
    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(path));
    EXPECT_EQ(r.samples(0, false), 5u);
    EXPECT_EQ(r.samples(1, false), 0u) << "a channel past its limit has no block";
}

TEST(BinChunk, IsOneBlockThatNoLongerNeedsTheDmaBuffer) {
    Arena arena(8, kBlock);
    auto pack = MakePack(arena, 0);
    BinFileState state;
    auto chunk = buildBINChunk(pack, {{DataLib::CH1, 32}, {DataLib::CH2, 32}}, state);
    const std::string before = Bytes(*chunk);
    ASSERT_EQ(chunk->blocks().size(), 1u);
    std::memset(pack->getBuffer(DataLib::CH1)->getMappedDataMemory(), 0x55, kBlock);  // the ring reuses the buffer
    EXPECT_EQ(Bytes(*chunk), before);
}

TEST(TdmsChunk, ReferencesTheBuffersAndReadsBack) {
    std::vector<int16_t> a{1, 2, 3};
    std::vector<int16_t> b{-1, -2, -3};
    std::map<DataLib::EDataBuffersPackChannel, SBuffPass> map{{DataLib::CH1, Pass(a)}, {DataLib::CH2, Pass(b)}};
    auto time = std::make_shared<std::vector<int64_t>>(std::vector<int64_t>{10, 18, 26});
    auto chunk = buildTDMSChunk(map, time);

    bool referenced = false;
    for (const auto& block : chunk->blocks())
        referenced |= block.data == map[DataLib::CH1].buffer.get();
    EXPECT_TRUE(referenced) << "the samples are not copied into the chunk";

    TDMS::FileReader f;
    ASSERT_TRUE(f.open(std::make_unique<std::stringstream>(Bytes(*chunk), std::ios::in | std::ios::binary))) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("Group", "ch1"), a);
    EXPECT_EQ(f.readAll<int16_t>("Group", "ch2"), b);
    EXPECT_EQ(f.readAll<int64_t>("Group", "time"), *time);
    EXPECT_EQ(f.channels()[0].name, "time") << "the column order of the former files";
}

// reader_lib plays TDMS files through TDMS::File; it has to read what the board records now
TEST(TdmsChunk, TheFormerReaderReadsIt) {
    std::vector<int16_t> a{5, 6};
    std::map<DataLib::EDataBuffersPackChannel, SBuffPass> map{{DataLib::CH1, Pass(a)}};
    const auto path = TempDir() + "/former.tdms";
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        buildTDMSChunk(map, nullptr)->writeTo(out);
        buildTDMSChunk(map, nullptr)->writeTo(out);
    }
    TDMS::File file;
    auto metadata = file.ReadFile(path);
    std::vector<int16_t> got;
    for (const auto& m : metadata) {
        if (m->PathStr != "/'Group'/'ch1'")
            continue;
        for (const auto& raw : m->RawData.DataType.GetRawVector()) {
            const auto* v = reinterpret_cast<const int16_t*>(raw->data.get());
            got.insert(got.end(), v, v + raw->size / 2);
        }
    }
    EXPECT_EQ(got, (std::vector<int16_t>{5, 6, 5, 6}));
}

TEST(WavChunk, TheHeaderComesWithTheFirstChunkThenInterleavedSamples) {
    std::vector<int16_t> a{1, 2};
    std::vector<int16_t> b{-1, -2};
    std::map<DataLib::EDataBuffersPackChannel, SBuffPass> map{{DataLib::CH1, Pass(a)}, {DataLib::CH2, Pass(b)}};
    CWaveWriter wav;
    const std::string first = Bytes(*wav.BuildWAVChunk(map));
    const std::string second = Bytes(*wav.BuildWAVChunk(map));
    ASSERT_EQ(first.size(), 44u + 8u);
    EXPECT_EQ(first.substr(0, 4), "RIFF");
    EXPECT_EQ(first.substr(36, 4), "data");
    uint32_t dataSize = 0;
    uint32_t rate = 0;
    uint16_t channels = 0;
    std::memcpy(&dataSize, first.data() + 40, 4);
    std::memcpy(&rate, first.data() + 24, 4);
    std::memcpy(&channels, first.data() + 22, 2);
    EXPECT_EQ(dataSize, 8u);
    EXPECT_EQ(rate, 125000000u);
    EXPECT_EQ(channels, 2u);
    std::vector<int16_t> samples(4);
    std::memcpy(samples.data(), second.data(), 8);
    EXPECT_EQ(second.size(), 8u);
    EXPECT_EQ(samples, (std::vector<int16_t>{1, -1, 2, -2}));
}

// 8 bit WAV samples are unsigned with 0x80 for zero (Microsoft RIFF/WAVE): the signed samples are
// offset by 0x80, a channel shorter than the others is filled with silence (0x80)
TEST(WavChunk, EightBitSamplesAreUnsignedWithZeroAt0x80) {
    const std::vector<int8_t> a{0, 127, -128, -1};
    const std::vector<int8_t> b{5};
    auto pass8 = [](const std::vector<int8_t>& x) {
        SBuffPass p;
        p.buffer = net_lib::createBuffer(reinterpret_cast<const char*>(x.data()), x.size());
        p.bufferLen = x.size();
        p.samplesCount = x.size();
        p.bitsBySample = 8;
        p.adcSpeed = 125000000;
        return p;
    };
    std::map<DataLib::EDataBuffersPackChannel, SBuffPass> map{{DataLib::CH1, pass8(a)}, {DataLib::CH2, pass8(b)}};
    CWaveWriter wav;
    const std::string first = Bytes(*wav.BuildWAVChunk(map));
    ASSERT_EQ(first.size(), 44u + 8u);
    uint16_t bits = 0;
    std::memcpy(&bits, first.data() + 34, 2);
    EXPECT_EQ(bits, 8u);
    const std::string data = first.substr(44);
    EXPECT_EQ(std::vector<uint8_t>(data.begin(), data.end()), (std::vector<uint8_t>{0x80, 0x85, 0xFF, 0x80, 0x00, 0x80, 0x7F, 0x80}));
}

class StreamingFileTest : public ::testing::TestWithParam<CStreamSettings::DataFormat::_enumerated> {};

// The whole way: packs into CStreamingFile, through the file thread, into the file
TEST_P(StreamingFileTest, PacksEndUpInTheFile) {
    const CStreamSettings::DataFormat format = GetParam();
    std::string dir = TempDir();
    const std::string path = dir + "/e2e_" + std::to_string(static_cast<int>(GetParam()));
    Arena arena(8, kBlock);
    auto file = streaming_lib::CStreamingFile::create(format, dir, 0, false, false);
    file->run("", path);
    for (int n = 0; n < 20; n++)
        ASSERT_EQ(file->passBuffers(MakePack(arena, n)), 1);
    file->stopAndFlush();

    std::vector<int16_t> ch1;
    for (int n = 0; n < 20; n++)
        for (int i = 0; i < 32; i++)
            ch1.push_back(static_cast<int16_t>(n * 100 + i));

    if (format.value == CStreamSettings::DataFormat::BIN) {
        bin_lib::FileReader r;
        ASSERT_TRUE(r.open(path)) << r.error();
        EXPECT_EQ(r.samples(0, false), 640u);
        EXPECT_EQ(r.samples(1, false), 640u);
        std::vector<int16_t> got(640);
        ASSERT_EQ(r.read(0, 0, 640, got.data(), false), 640u);
        EXPECT_EQ(got, ch1);
    } else if (format.value == CStreamSettings::DataFormat::TDMS) {
        TDMS::FileReader f;
        ASSERT_TRUE(f.open(path)) << f.error();
        EXPECT_EQ(f.readAll<int16_t>("Group", "ch1"), ch1);
        EXPECT_EQ(f.channel("Group", "ch2")->count, 640u);
        EXPECT_EQ(f.channel("Group", "time"), nullptr) << "no time column by default";
        // the time is in the properties of the channels and in a row per buffer
        EXPECT_EQ(TDMS::getProperty<double>(f.groupProperties("Group"), "sample_rate"), 125e6);
        EXPECT_EQ(TDMS::getProperty<std::string>(f.groupProperties("Group"), "data_type"), std::string("Raw"));
        auto wf = TDMS::waveformOf(*f.channel("Group", "ch1"));
        ASSERT_TRUE(wf);
        EXPECT_EQ(wf->start.unixNs(), 1000000);
        EXPECT_DOUBLE_EQ(wf->increment, 8e-9);
        std::vector<int64_t> starts;
        for (int n = 0; n < 20; n++)
            starts.push_back(1000000 + n);
        EXPECT_EQ(f.readAll<int64_t>("Buffers", "start_time_ns"), starts);
        EXPECT_EQ(f.readAll<uint64_t>("Buffers", "samples"), std::vector<uint64_t>(20, 32));
        EXPECT_EQ(f.readAll<uint64_t>("Buffers", "lost"), std::vector<uint64_t>(20, 0));
        EXPECT_EQ(f.channels()[0].name, "ch1") << "the samples come first in every segment";
    } else {
        const std::string bytes = ReadFile(path);
        ASSERT_EQ(bytes.size(), 44u + 640u * 2u * 2u);
        uint32_t dataSize = 0;
        std::memcpy(&dataSize, bytes.data() + 40, 4);
        EXPECT_EQ(dataSize, 640u * 2u * 2u) << "the header counts every chunk";
        int16_t last[2];
        std::memcpy(last, bytes.data() + bytes.size() - 4, 4);
        EXPECT_EQ(last[0], 1931);
        EXPECT_EQ(last[1], -1931);
    }
}

TEST(StreamingFile, TheTdmsTimeColumnIsThereWhenAskedFor) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_column.tdms";
    Arena arena(8, kBlock);
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::TDMS, dir, 0, false, false);
    file->setTDMSTimeColumn(true);
    file->run("", path);
    for (int n = 0; n < 3; n++)
        file->passBuffers(MakePack(arena, n));
    file->stopAndFlush();
    TDMS::FileReader f;
    ASSERT_TRUE(f.open(path)) << f.error();
    ASSERT_NE(f.channel("Group", "time"), nullptr);
    EXPECT_EQ(f.channel("Group", "time")->count, 96u);
    EXPECT_EQ(f.read<int64_t>("Group", "time", 32, 2), (std::vector<int64_t>{1000001, 1000009})) << "every segment its own time, 8 ns a sample";
}

// With a sample limit the channels stop in the middle of a pack: the time column stops with them
TEST(StreamingFile, TheTdmsTimeColumnStopsAtTheSampleLimit) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_limit.tdms";
    Arena arena(8, kBlock);
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::TDMS, dir, 40, false, false);
    file->setTDMSTimeColumn(true);
    file->run("", path);
    for (int n = 0; n < 3; n++)
        file->passBuffers(MakePack(arena, n));
    file->stopAndFlush();
    TDMS::FileReader f;
    ASSERT_TRUE(f.open(path)) << f.error();
    ASSERT_NE(f.channel("Group", "ch1"), nullptr);
    ASSERT_NE(f.channel("Group", "time"), nullptr);
    EXPECT_EQ(f.channel("Group", "ch1")->count, 40u);
    EXPECT_EQ(f.channel("Group", "time")->count, 40u) << "a time per sample written, not per sample of the packs";
}

// 62.5 MHz / 64: the integer rate of the pack is 976562, the header has the ADC clock and the
// decimation, the file the exact rate
TEST(StreamingFile, TheTdmsRateIsExactFromTheAdcClockAndTheDecimation) {
    std::string dir = TempDir();
    const std::string path = dir + "/exact_rate.tdms";
    Arena arena(8, kBlock);
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::TDMS, dir, 0, false, false);
    file->setTDMSTimeColumn(true);
    file->run("", path);
    for (int n = 0; n < 2; n++) {
        auto pack = MakePack(arena, n);
        pack->setOSCRate(976562);
        for (auto ch : {DataLib::CH1, DataLib::CH2}) {
            pack->getBuffer(ch)->setADCClock(62500000);
            pack->getBuffer(ch)->setDecimation(64);
        }
        file->passBuffers(pack);
    }
    file->stopAndFlush();
    TDMS::FileReader f;
    ASSERT_TRUE(f.open(path)) << f.error();
    EXPECT_EQ(TDMS::getProperty<double>(f.groupProperties("Group"), "sample_rate"), 976562.5);
    EXPECT_EQ(TDMS::getProperty<double>(f.groupProperties("Group"), "adc_rate"), 62.5e6);
    EXPECT_EQ(TDMS::getProperty<uint32_t>(f.groupProperties("Group"), "decimation"), 64u);
    auto wf = TDMS::waveformOf(*f.channel("Group", "ch1"));
    ASSERT_TRUE(wf);
    EXPECT_DOUBLE_EQ(wf->increment, 1.024e-6);
    EXPECT_EQ(f.read<int64_t>("Group", "time", 31, 1), (std::vector<int64_t>{1000000 + 31744})) << "31 x 1024 ns";
}

INSTANTIATE_TEST_SUITE_P(Formats, StreamingFileTest,
                         ::testing::Values(CStreamSettings::DataFormat::BIN, CStreamSettings::DataFormat::TDMS, CStreamSettings::DataFormat::WAV));

// ---- limits and parts (streaming_improvements 4.11)

namespace {

// the files of a recording in parts: <base>_0001.<ext> ...
auto Parts(const std::string& base, const std::string& ext) -> std::vector<std::string> {
    std::vector<std::string> r;
    for (int i = 1;; i++) {
        char n[16];
        snprintf(n, sizeof(n), "_%04d", i);
        const std::string p = base + n + "." + ext;
        if (!std::filesystem::exists(p))
            return r;
        r.push_back(p);
    }
}

auto Channel1(int packs) -> std::vector<int16_t> {
    std::vector<int16_t> ch1;
    for (int n = 0; n < packs; n++)
        for (int i = 0; i < 32; i++)
            ch1.push_back(static_cast<int16_t>(n * 100 + i));
    return ch1;
}

// the recording of 20 packs of 32 samples of two channels into `file`; the stops it reported
// refuseAt: the chunk of that pack refused as by a full queue
auto Record(streaming_lib::CStreamingFile::Ptr file, const std::string& path, int packs, std::vector<int>* stops = nullptr, int refuseAt = -1) -> void {
    Arena arena(8, kBlock);
    if (stops)
        file->stopNotify.connect([stops](streaming_lib::CStreamingFile::EStopReason r) { stops->push_back(static_cast<int>(r)); });
    file->run("", path);
    for (int n = 0; n < packs; n++) {
        if (n == refuseAt)
            file->refuseChunks(1);
        ASSERT_EQ(file->passBuffers(MakePack(arena, n)), 1);
    }
    file->stopAndFlush();
}

// Channel1 without pack `gone`
auto Channel1Without(int packs, int gone) -> std::vector<int16_t> {
    auto v = Channel1(packs);
    v.erase(v.begin() + gone * 32, v.begin() + (gone + 1) * 32);
    return v;
}

}  // namespace

// BIN in parts of about 5 packs: every part is a BIN of its own with its time line from 0 and the
// time of its first buffer; together they hold every sample; each has its report
TEST(StreamingFileParts, ABinInPartsOfASizeHoldsEverySampleOnce) {
    std::string dir = TempDir();
    const std::string base = dir + "/split_bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    // a pack is 2 blocks of 64 bytes and a block header each: 5 of them and a file header
    file->setSplit(5 * 2 * (64 + 48) + 200, 0);
    Record(file, base + ".bin", 20);
    const auto parts = Parts(base, "bin");
    ASSERT_EQ(parts.size(), 4u);
    EXPECT_FALSE(std::filesystem::exists(base + ".bin")) << "numbered from the first part";
    EXPECT_EQ(file->getPart(), 4);
    EXPECT_EQ(file->getCSVFileName(), parts.back());
    std::vector<int16_t> all;
    for (size_t k = 0; k < parts.size(); k++) {
        bin_lib::FileReader r;
        ASSERT_TRUE(r.open(parts[k])) << parts[k] << ": " << r.error();
        EXPECT_EQ(r.info().startTime, 1000000 + static_cast<int64_t>(k) * 5) << "the time of its first buffer";
        bin_lib::BlockInfo b;
        std::vector<uint8_t> d;
        ASSERT_TRUE(r.nextBlock(b, d));
        EXPECT_EQ(b.position, 0u) << "the time line of a part from 0";
        const uint64_t n = r.samples(0, false);
        EXPECT_EQ(n, 160u) << parts[k];
        std::vector<int16_t> got(n);
        r.read(0, 0, n, got.data(), false);
        all.insert(all.end(), got.begin(), got.end());
        EXPECT_TRUE(std::filesystem::exists(parts[k] + ".log.txt")) << "the report of " << parts[k];
        EXPECT_TRUE(std::filesystem::exists(parts[k] + ".log.lost.txt"));
    }
    EXPECT_EQ(all, Channel1(20));
    EXPECT_EQ(file->getPassSamples(), 640u) << "the whole recording";
    EXPECT_EQ(file->getWritedSize(), [&] { uint64_t s = 0; for (auto& p : parts) s += std::filesystem::file_size(p); return s; }());
}

// WAV in parts of a time: every part has its own header with its own sizes
TEST(StreamingFileParts, AWavInPartsOfATimeHasAHeaderInEveryPart) {
    std::string dir = TempDir();
    const std::string base = dir + "/split_wav";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::WAV, dir, 0, false, false);
    file->setSplit(0, 32.0 * 6 / 125e6);  // 6 packs of the time line
    Record(file, base + ".wav", 20);
    const auto parts = Parts(base, "wav");
    ASSERT_EQ(parts.size(), 4u);  // 6 + 6 + 6 + 2
    std::vector<int16_t> all;
    const int packs[4] = {6, 6, 6, 2};
    for (size_t k = 0; k < parts.size(); k++) {
        const std::string bytes = ReadFile(parts[k]);
        ASSERT_EQ(bytes.size(), 44u + packs[k] * 32u * 2u * 2u) << parts[k];
        EXPECT_EQ(bytes.substr(0, 4), "RIFF");
        uint32_t data = 0, riff = 0;
        std::memcpy(&riff, bytes.data() + 4, 4);
        std::memcpy(&data, bytes.data() + 40, 4);
        EXPECT_EQ(data, bytes.size() - 44) << parts[k];
        EXPECT_EQ(riff, bytes.size() - 8) << parts[k];
        for (size_t i = 44; i + 4 <= bytes.size(); i += 4) {
            int16_t v;
            std::memcpy(&v, bytes.data() + i, 2);
            all.push_back(v);
        }
    }
    EXPECT_EQ(all, Channel1(20));
}

TEST(StreamingFileParts, ATdmsInPartsReadsBackPartByPart) {
    std::string dir = TempDir();
    const std::string base = dir + "/split_tdms";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::TDMS, dir, 0, false, false);
    file->setSplit(4000, 0);
    Record(file, base + ".tdms", 20);
    const auto parts = Parts(base, "tdms");
    ASSERT_GE(parts.size(), 2u);
    std::vector<int16_t> all;
    for (const auto& p : parts) {
        TDMS::FileReader f;
        ASSERT_TRUE(f.open(p)) << p << ": " << f.error();
        EXPECT_EQ(TDMS::getProperty<double>(f.groupProperties("Group"), "sample_rate"), 125e6) << "the properties again in every part";
        const auto ch1 = f.readAll<int16_t>("Group", "ch1");
        all.insert(all.end(), ch1.begin(), ch1.end());
        EXPECT_LE(std::filesystem::file_size(p), 4000u + 2000u) << "about the size asked for";
    }
    EXPECT_EQ(all, Channel1(20));
}

// The first chunk of a part refused (the queue full when the card is slow): the next chunk
// carries the header of the part, which stays a whole file of its format
TEST(StreamingFileParts, ABinPartWhoseFirstChunkIsRefusedStillHasItsHeader) {
    std::string dir = TempDir();
    const std::string base = dir + "/refused_bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setSplit(5 * 2 * (64 + 48) + 200, 0);
    Record(file, base + ".bin", 20, nullptr, 5);  // pack 5: the first of part 2
    const auto parts = Parts(base, "bin");
    ASSERT_GE(parts.size(), 3u);
    std::vector<int16_t> all;
    for (const auto& p : parts) {
        bin_lib::FileReader r;
        ASSERT_TRUE(r.open(p)) << p << ": " << r.error();
        const uint64_t n = r.samples(0, false);
        std::vector<int16_t> got(n);
        r.read(0, 0, n, got.data(), false);
        all.insert(all.end(), got.begin(), got.end());
    }
    EXPECT_EQ(all, Channel1Without(20, 5));
}

TEST(StreamingFileParts, AWavPartWhoseFirstChunkIsRefusedStillHasItsHeader) {
    std::string dir = TempDir();
    const std::string base = dir + "/refused_wav";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::WAV, dir, 0, false, false);
    file->setSplit(0, 32.0 * 6 / 125e6);
    Record(file, base + ".wav", 20, nullptr, 6);  // pack 6: the first of part 2
    const auto parts = Parts(base, "wav");
    ASSERT_EQ(parts.size(), 4u);
    std::vector<int16_t> all;
    for (const auto& p : parts) {
        const std::string bytes = ReadFile(p);
        ASSERT_GE(bytes.size(), 44u) << p;
        EXPECT_EQ(bytes.substr(0, 4), "RIFF") << p;
        uint32_t data = 0, riff = 0;
        std::memcpy(&riff, bytes.data() + 4, 4);
        std::memcpy(&data, bytes.data() + 40, 4);
        EXPECT_EQ(data, bytes.size() - 44) << p;
        EXPECT_EQ(riff, bytes.size() - 8) << p;
        for (size_t i = 44; i + 4 <= bytes.size(); i += 4) {
            int16_t v;
            std::memcpy(&v, bytes.data() + i, 2);
            all.push_back(v);
        }
    }
    EXPECT_EQ(all, Channel1Without(20, 6));
}

TEST(StreamingFileParts, ATdmsPartWhoseFirstChunkIsRefusedStillHasItsProperties) {
    std::string dir = TempDir();
    const std::string base = dir + "/refused_tdms";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::TDMS, dir, 0, false, false);
    file->setSplit(4000, 0);
    // the first part: as many packs as the split takes
    {
        auto probe = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::TDMS, dir, 0, false, false);
        probe->setSplit(4000, 0);
        Record(probe, dir + "/probe_tdms.tdms", 20);
    }
    TDMS::FileReader first;
    ASSERT_TRUE(first.open(dir + "/probe_tdms_0001.tdms")) << first.error();
    const int firstPacks = static_cast<int>(first.readAll<int16_t>("Group", "ch1").size() / 32);
    ASSERT_GT(firstPacks, 0);
    Record(file, base + ".tdms", 20, nullptr, firstPacks);
    const auto parts = Parts(base, "tdms");
    ASSERT_GE(parts.size(), 2u);
    std::vector<int16_t> all;
    for (const auto& p : parts) {
        TDMS::FileReader f;
        ASSERT_TRUE(f.open(p)) << p << ": " << f.error();
        EXPECT_EQ(TDMS::getProperty<double>(f.groupProperties("Group"), "sample_rate"), 125e6) << p;
        const auto ch1 = f.readAll<int16_t>("Group", "ch1");
        all.insert(all.end(), ch1.begin(), ch1.end());
    }
    EXPECT_EQ(all, Channel1Without(20, firstPacks));
}

// The size limit at a part boundary: no part is begun for the pack it keeps out
TEST(StreamingFileParts, NoEmptyPartIsBegunForAPackTheSizeLimitKeepsOut) {
    std::string dir = TempDir();
    const std::string base = dir + "/limit_split";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setSplit(5 * 2 * (64 + 48) + 200, 0);  // 5 packs a part
    // just under 2 parts: the pack that begins part 3 stays out
    uint64_t two = 0;
    {
        auto probe = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
        probe->setSplit(5 * 2 * (64 + 48) + 200, 0);
        Record(probe, dir + "/probe_limit.bin", 10);
        two = probe->getWritedSize();
    }
    file->setLimits(0, two + 100);
    std::vector<int> stops;
    Record(file, base + ".bin", 20, &stops);
    EXPECT_EQ(Parts(base, "bin").size(), 2u) << "no third part";
    ASSERT_FALSE(stops.empty());
    EXPECT_EQ(stops[0], streaming_lib::CStreamingFile::REACH_LIMIT);
    EXPECT_LE(file->getWritedSize(), two + 100);
}

// a WAV without parts asked for goes on in a new part before its header would overflow: the
// first file keeps its name
TEST(StreamingFileParts, AWavGoesOnInANewPartBeforeItsHeaderOverflows) {
    std::string dir = TempDir();
    const std::string base = dir + "/wav_max";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::WAV, dir, 0, false, false);
    file->setWavPartMax(1000);  // 4 GB in the field
    Record(file, base + ".wav", 10);
    ASSERT_TRUE(std::filesystem::exists(base + ".wav"));
    ASSERT_TRUE(std::filesystem::exists(base + "_0002.wav"));
    EXPECT_FALSE(std::filesystem::exists(base + "_0001.wav"));
    EXPECT_LE(std::filesystem::file_size(base + ".wav"), 1000u);
    uint64_t samples = 0;
    for (int k = 1;; k++) {
        char n[16];
        snprintf(n, sizeof(n), "_%04d", k);
        const std::string p = k == 1 ? base + ".wav" : base + n + ".wav";
        if (!std::filesystem::exists(p))
            break;
        samples += (std::filesystem::file_size(p) - 44) / 4;
    }
    EXPECT_EQ(samples, 320u) << "every sample in one of them";
}

// The real 4 GB of a WAV header (kWavPartMax): over 4 GB of data in one WAV goes on in _0002,
// both parts readable with their own sizes, every frame once. Writes about 4.3 GB: only with
// RP_BIG_TESTS=1
TEST(StreamingFileParts, AWavOverFourGigabytesGoesOnInASecondPart) {
    if (!std::getenv("RP_BIG_TESTS"))
        GTEST_SKIP() << "RP_BIG_TESTS=1 to write 4.3 GB";
    std::string dir = TempDir();
    const std::string base = dir + "/wav_4g";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::WAV, dir, 0, false, false);
    constexpr size_t kBig = 1024 * 1024;  // 512 k samples of 16 bit a channel: 2 MB of WAV a pack
    Arena arena(8, kBig);
    file->run("", base + ".wav");
    const int packs = static_cast<int>((streaming_lib::CStreamingFile::kWavPartMax + 200ull * 1024 * 1024) / (2 * kBig));
    for (int n = 0; n < packs; n++) {
        // as fast as the disk: the queue never refuses
        while (file->getQueueFill() > 0.5)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ASSERT_EQ(file->passBuffers(MakePack(arena, n)), 1);
    }
    file->stopAndFlush();
    ASSERT_EQ(file->getDroppedSamples(), 0u) << "the queue refused some";
    const std::string first = base + ".wav", second = base + "_0002.wav";
    ASSERT_TRUE(std::filesystem::exists(first));
    ASSERT_TRUE(std::filesystem::exists(second));
    EXPECT_FALSE(std::filesystem::exists(base + "_0003.wav"));
    uint64_t frames = 0;
    for (const auto& p : {first, second}) {
        const uint64_t size = std::filesystem::file_size(p);
        EXPECT_LT(size, 0xFFFFFFFFull) << p;
        std::ifstream in(p, std::ios::binary);
        char h[44];
        in.read(h, 44);
        uint32_t riff = 0, data = 0;
        std::memcpy(&riff, h + 4, 4);
        std::memcpy(&data, h + 40, 4);
        EXPECT_EQ(std::string(h, 4), "RIFF") << p;
        EXPECT_EQ(riff, size - 8) << p;
        EXPECT_EQ(data, size - 44) << p;
        frames += (size - 44) / 4;
    }
    EXPECT_EQ(frames, static_cast<uint64_t>(packs) * (kBig / 2)) << "every frame once";
    // the boundary: the last frame of the first part, the first of the second follow each other
    const uint64_t firstFrames = (std::filesystem::file_size(first) - 44) / 4;
    std::ifstream a(first, std::ios::binary), b(second, std::ios::binary);
    a.seekg(static_cast<std::streamoff>(44 + (firstFrames - 1) * 4));
    b.seekg(44);
    int16_t last = 0, next = 0;
    a.read(reinterpret_cast<char*>(&last), 2);
    b.read(reinterpret_cast<char*>(&next), 2);
    const uint64_t n = firstFrames / (kBig / 2);  // the pack the second part starts with
    EXPECT_EQ(firstFrames % (kBig / 2), 0u) << "a part ends at a pack";
    EXPECT_EQ(last, static_cast<int16_t>((n - 1) * 100 + (kBig / 2 - 1)));
    EXPECT_EQ(next, static_cast<int16_t>(n * 100));
    std::filesystem::remove(first);
    std::filesystem::remove(second);
}

TEST(StreamingFileLimits, TheTimeLimitStopsAtTheBufferThatReachesIt) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_limit.bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setLimits(32.0 * 7 / 125e6, 0);  // 7 packs
    std::vector<int> stops;
    Record(file, path, 20, &stops);
    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(path)) << r.error();
    EXPECT_EQ(r.samples(0, false), 7u * 32u);
    ASSERT_FALSE(stops.empty());
    EXPECT_EQ(stops[0], streaming_lib::CStreamingFile::REACH_LIMIT);
}

TEST(StreamingFileLimits, TheSizeLimitIsNeverPassed) {
    std::string dir = TempDir();
    const std::string path = dir + "/size_limit.wav";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::WAV, dir, 0, false, false);
    file->setLimits(0, 1000);
    std::vector<int> stops;
    Record(file, path, 20, &stops);
    const auto size = std::filesystem::file_size(path);
    EXPECT_LE(size, 1000u);
    EXPECT_GE(size, 1000u - 2 * 32 * 2 * 2 - 64) << "up to the last whole pack";
    ASSERT_FALSE(stops.empty());
    EXPECT_EQ(stops[0], streaming_lib::CStreamingFile::REACH_LIMIT);
}

// a trigger after a wait longer than the limit: its data is past the limit, nothing of it written
TEST(StreamingFileLimits, APackWhoseDataStartsPastTheTimeLimitIsNotWritten) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_past.bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setLimits(32.0 * 10 / 125e6, 0);  // 10 packs of the time line
    std::vector<int> stops;
    file->stopNotify.connect([&stops](streaming_lib::CStreamingFile::EStopReason r) { stops.push_back(static_cast<int>(r)); });
    Arena arena(8, kBlock);
    file->run("", path);
    file->passBuffers(MakePack(arena, 0));
    auto late = MakePack(arena, 1);
    for (int c = 0; c < 2; c++)
        late->getBuffer(c ? DataLib::CH2 : DataLib::CH1)->setLostSamples(DataLib::WAIT, 32 * 20);
    file->passBuffers(late);
    file->stopAndFlush();
    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(path)) << r.error();
    EXPECT_EQ(r.samples(0, false), 32u) << "the first pack only";
    ASSERT_FALSE(stops.empty());
    EXPECT_EQ(stops[0], streaming_lib::CStreamingFile::REACH_LIMIT);
}

// a trigger that never comes: no buffer ends the time limit, the clock does
TEST(StreamingFileLimits, TheTimeLimitEndsTheRecordingWhenNoBufferComes) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_clock.bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setLimits(0.05, 0, 0.01);
    file->setTimeMargin(0.05);
    std::atomic<int> stops{0};
    std::atomic<int> reason{-1};
    file->stopNotify.connect([&](streaming_lib::CStreamingFile::EStopReason r) {
        reason = static_cast<int>(r);
        stops++;
    });
    Arena arena(8, kBlock);
    file->run("", path);
    file->passBuffers(MakePack(arena, 0));  // then nothing: the FPGA waits for a trigger
    for (int i = 0; i < 100 && stops == 0; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_EQ(stops, 1) << "stopped by the clock";
    EXPECT_EQ(reason, streaming_lib::CStreamingFile::REACH_LIMIT);
    EXPECT_EQ(file->passBuffers(MakePack(arena, 1)), 1);
    file->stopAndFlush();
    EXPECT_EQ(stops, 1) << "once";
    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(path)) << r.error();
    EXPECT_EQ(r.samples(0, false), 32u) << "what came before: whole; nothing after";
}

// buffers that keep coming, late on the clock (the ring behind a slow network or card): the clock
// does not cut them, the time line ends the limit
TEST(StreamingFileLimits, TheClockDoesNotCutBuffersThatStillCome) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_clock_late.bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setLimits(0.05, 0, 0.01);  // the packs are far shorter on the time line: never reached
    file->setTimeMargin(0.05);
    std::atomic<int> stops{0};
    file->stopNotify.connect([&](streaming_lib::CStreamingFile::EStopReason) { stops++; });
    Arena arena(8, kBlock);
    file->run("", path);
    for (int n = 0; n < 15; n++) {
        file->passBuffers(MakePack(arena, n));
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    EXPECT_EQ(stops, 0) << "0.3 s past the start, but a buffer every 20 ms";
    file->stopAndFlush();
    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(path)) << r.error();
    EXPECT_EQ(r.samples(0, false), 15u * 32u);
}

// buffers that come in time end the limit by themselves; the clock waits for a long buffer
TEST(StreamingFileLimits, TheClockWaitsForABufferThatLastsLong) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_clock_long.bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setLimits(0.01, 0, 0.5);  // a buffer of 0.5 s (a large decimation)
    file->setTimeMargin(0.01);
    std::atomic<int> stops{0};
    file->stopNotify.connect([&](streaming_lib::CStreamingFile::EStopReason) { stops++; });
    file->run("", path);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(stops, 0) << "the first buffer may still come";
    for (int i = 0; i < 100 && stops == 0; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(stops, 1) << "after the limit, the buffer and the margin";
    file->stopAndFlush();
}

TEST(StreamingFileLimits, TheTimeLimitCountsTheTriggerWaits) {
    std::string dir = TempDir();
    const std::string path = dir + "/time_waits.bin";
    auto file = streaming_lib::CStreamingFile::create(CStreamSettings::DataFormat::BIN, dir, 0, false, false);
    file->setLimits(32.0 * 7 / 125e6, 0);  // 7 packs of the time line
    Arena arena(8, kBlock);
    file->run("", path);
    for (int n = 0; n < 20; n++) {
        auto pack = MakePack(arena, n);
        // every pack after a wait as long as itself: the time line goes twice as fast
        for (int c = 0; c < 2; c++)
            pack->getBuffer(c ? DataLib::CH2 : DataLib::CH1)->setLostSamples(DataLib::WAIT, 32);
        file->passBuffers(pack);
    }
    file->stopAndFlush();
    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(path)) << r.error();
    // the data of pack k starts at 64k + 32: the 4th at 7 x 32, where the limit is, stays out
    EXPECT_EQ(r.samples(0, false), 3u * 32u) << "3 packs, the 4th starts at the limit";
}
