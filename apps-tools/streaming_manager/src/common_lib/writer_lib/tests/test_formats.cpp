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
#include <cstring>
#include <filesystem>
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
