/**
 * The segments of the recording formats and CStreamingFile writing them.
 *
 * BIN keeps its bytes (convert_tool reads them): the chunk is compared with the former
 * stringstream builder, kept here as the reference. TDMS segments reference the converted buffers
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

// The former buildBINStream, as the reference for the BIN bytes
auto FormerBIN(DataLib::CDataBuffersPackDMA::Ptr pack, std::map<DataLib::EDataBuffersPackChannel, uint32_t> samples) -> std::string {
    static constexpr uint8_t end[12] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    std::ostringstream memory(std::ios::binary);
    CBinInfo::BinHeader header;
    std::memset(static_cast<void*>(&header), 0, sizeof(header));  // the padding, as buildBINChunk writes it
    DataLib::CDataBufferDMA::Ptr ch[4] = {pack->getBuffer(DataLib::CH1), pack->getBuffer(DataLib::CH2), pack->getBuffer(DataLib::CH3), pack->getBuffer(DataLib::CH4)};
    uint32_t size[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
        if (ch[i]) {
            uint32_t s = std::min<uint32_t>(ch[i]->getSamplesCount(), samples[(DataLib::EDataBuffersPackChannel)i]);
            size[i] = std::min<uint32_t>(s * (ch[i]->getBitBySample() / 8), ch[i]->getDataLenght());
            header.dataFormatSize[i] = ch[i]->getBitBySample() / 8;
            header.sizeCh[i] = size[i];
            header.sampleCh[i] = s;
            header.lostCount[i] = ch[i]->getLostSamples(DataLib::FPGA);
            header.oscRate[i] = ch[i]->getADCBaseRate();
            header.timeCapture[i] = ch[i]->getTimeCapture();
        }
    }
    header.sigmentLength = size[0] + size[1] + size[2] + size[3];
    memory.write(reinterpret_cast<const char*>(&header), sizeof(header));
    for (int i = 0; i < 4; i++)
        if (ch[i] && size[i])
            memory.write(static_cast<const char*>(ch[i]->getMappedDataMemory()), size[i]);
    memory.write(reinterpret_cast<const char*>(end), 12);
    return memory.str();
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

TEST(BinChunk, KeepsTheBytesOfTheFormerBuilder) {
    Arena arena(8, kBlock);
    auto pack = MakePack(arena, 3, 7);
    std::map<DataLib::EDataBuffersPackChannel, uint32_t> all{{DataLib::CH1, 32}, {DataLib::CH2, 32}};
    EXPECT_EQ(Bytes(*buildBINChunk(pack, all)), FormerBIN(pack, all));
    std::map<DataLib::EDataBuffersPackChannel, uint32_t> limited{{DataLib::CH1, 5}, {DataLib::CH2, 32}};
    EXPECT_EQ(Bytes(*buildBINChunk(pack, limited)), FormerBIN(pack, limited)) << "a sample limit";
}

TEST(BinChunk, ThePaddingOfTheHeaderIsZero) {
    Arena arena(8, kBlock);
    auto pack = MakePack(arena, 1, 3);
    const std::string bytes = Bytes(*buildBINChunk(pack, {{DataLib::CH1, 32}, {DataLib::CH2, 32}}));
    const auto padBeforeLost = offsetof(CBinInfo::BinHeader, lostCount) - (offsetof(CBinInfo::BinHeader, sampleCh) + sizeof(uint32_t) * 4);
    const auto padAtEnd = sizeof(CBinInfo::BinHeader) - (offsetof(CBinInfo::BinHeader, sigmentLength) + sizeof(uint32_t));
    EXPECT_EQ(bytes.substr(offsetof(CBinInfo::BinHeader, lostCount) - padBeforeLost, padBeforeLost), std::string(padBeforeLost, '\0'));
    EXPECT_EQ(bytes.substr(sizeof(CBinInfo::BinHeader) - padAtEnd, padAtEnd), std::string(padAtEnd, '\0'));
}

TEST(BinChunk, IsOneBlockThatNoLongerNeedsTheDmaBuffer) {
    Arena arena(8, kBlock);
    auto pack = MakePack(arena, 0);
    auto chunk = buildBINChunk(pack, {{DataLib::CH1, 32}, {DataLib::CH2, 32}});
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
        std::fstream in(path, std::ios::in | std::ios::binary);
        auto info = readBinInfo(&in);
        EXPECT_EQ(info.segCount, 20u);
        EXPECT_EQ(info.samples_ch[0], 640u);
        EXPECT_EQ(info.samples_ch[1], 640u);
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

INSTANTIATE_TEST_SUITE_P(Formats, StreamingFileTest,
                         ::testing::Values(CStreamSettings::DataFormat::BIN, CStreamSettings::DataFormat::TDMS, CStreamSettings::DataFormat::WAV));
