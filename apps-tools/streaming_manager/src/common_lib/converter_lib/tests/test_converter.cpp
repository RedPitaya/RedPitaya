/**
 * The BIN stream file to CSV, as rpsa_client records CSV: a row per sample, an index column, the
 * lost samples as "-", the time of a sample from the start of the recording and its position.
 */

#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "bin_lib/bin_file.h"
#include "bin_lib/gpio_words.h"
#include "converter_lib/converter.h"
#include "tdms_lib/tdms.h"

namespace {

auto Path(const std::string& name) -> std::string {
    auto dir = std::filesystem::temp_directory_path() / ("converter_" + std::to_string(::getpid()));
    std::filesystem::create_directories(dir);
    return (dir / name).string();
}

auto ReadText(const std::string& path) -> std::string {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Two int16 channels, two packs of 2 samples; the second pack after 1 lost sample
auto WriteRecording(const std::string& path, int64_t start = 0) -> void {
    bin_lib::FileInfo info;
    info.clock = 125000000;
    info.decimation = 125;  // 1 us a sample
    info.startTime = start;
    info.channels = {{1, bin_lib::SampleType::INT16, 0, 14}, {2, bin_lib::SampleType::INT16, 0, 14}};
    auto out = bin_lib::fileHeader(info);
    bin_lib::Timeline line;
    for (int p = 0; p < 2; p++) {
        for (uint8_t c = 0; c < 2; c++) {
            const int16_t s[2] = {static_cast<int16_t>((c ? -1 : 1) * (p * 10 + 1)), static_cast<int16_t>((c ? -1 : 1) * (p * 10 + 2))};
            bin_lib::BlockInfo b;
            b.channel = c;
            b.samples = 2;
            b.lost = p;
            b.position = line.next(c, b.lost, 0, 2);
            bin_lib::appendBlock(out, b, s, sizeof(s));
        }
    }
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
}

}  // namespace

TEST(ConvertToCSV, ARowPerSampleWithTheIndexAndTheLostSamples) {
    const auto bin = Path("rec.bin");
    WriteRecording(bin);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToCSV(bin, "", FH_CSV_ADD_INDEX));
    EXPECT_EQ(ReadText(Path("rec.csv")), "1,1,-1\n2,2,-2\n3,-,-\n4,11,-11\n5,12,-12\n") << "the lost sample where it was lost, before its pack";
}

TEST(ConvertToCSV, TheTimeComesFromTheStartAndThePosition) {
    const auto bin = Path("time.bin");
    WriteRecording(bin, 1000000000);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToCSV(bin, "", static_cast<FH_CSVMode>(FH_CSV_ADD_TIME_COL_NS)));
    EXPECT_EQ(ReadText(Path("time.csv")), "1000000000,1,-1\n1000001000,2,-2\n1000002000,-,-\n1000003000,11,-11\n1000004000,12,-12\n")
        << "the second pack starts at position 3, after the lost sample at 2";
}

TEST(ConvertToCSV, TheSegmentsArePacks) {
    const auto bin = Path("seg.bin");
    WriteRecording(bin);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToCSV(bin, 2, 2, "", FH_CSV_ADD_INDEX));
    EXPECT_EQ(ReadText(Path("seg.csv")), "3,-,-\n4,11,-11\n5,12,-12\n") << "the index counts the skipped pack";
}

TEST(ConvertToCSV, NotABinStreamFile) {
    const auto bin = Path("bad.bin");
    std::ofstream(bin) << "old segments";
    converter_lib::CConverter c;
    EXPECT_FALSE(c.convertToCSV(bin, "", FH_CSV_ADD_INDEX));
}

TEST(ConvertToTDMS, TheSamplesAndTheRateOfTheRecording) {
    const auto bin = Path("tdms.bin");
    WriteRecording(bin, 1000000000);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToTDMS(bin, ""));
    TDMS::FileReader f;
    ASSERT_TRUE(f.open(Path("tdms.tdms"))) << f.error();
    // the lost sample of a buffer is a 0 before its samples, where it was lost (as in BIN and CSV)
    EXPECT_EQ(f.readAll<int16_t>("Group", "ch1"), (std::vector<int16_t>{1, 2, 0, 11, 12}));
    EXPECT_EQ(f.readAll<int16_t>("Group", "ch2"), (std::vector<int16_t>{-1, -2, 0, -11, -12}));
    EXPECT_EQ(TDMS::getProperty<double>(f.groupProperties("Group"), "sample_rate"), 1e6);
    EXPECT_EQ(f.readAll<uint64_t>("Buffers", "lost"), (std::vector<uint64_t>{0, 1}));
}

TEST(ConvertToWAV, TheSamplesInterleaved) {
    const auto bin = Path("wav.bin");
    WriteRecording(bin);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToWAV(bin, ""));
    std::ifstream in(Path("wav.wav"), std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(bytes.size(), 44u + 10u * 2u) << "the lost sample is a 0 frame";
    std::vector<int16_t> s(10);
    std::memcpy(s.data(), bytes.data() + 44, 20);
    EXPECT_EQ(s, (std::vector<int16_t>{1, -1, 2, -2, 0, 0, 11, -11, 12, -12})) << "before the buffer it was lost before";
}

namespace {

auto WriteGpio(const std::string& path) -> void {
    bin_lib::FileInfo info;
    info.source = bin_lib::Source::GPIO_IN;
    info.clock = 125000000;
    info.channels = {{1, bin_lib::SampleType::GPIO_IN_WORD, bin_lib::CHANNEL_FLAG_RLE, 0}};
    auto out = bin_lib::fileHeader(info);
    const uint32_t words[2] = {bin_lib::inWord(0x01, 2, 0x10, 2), bin_lib::inWord(0x02, 0, 0x20, 0)};
    bin_lib::BlockInfo b;
    b.samples = 4;
    bin_lib::appendBlock(out, b, words, sizeof(words));
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
}

}  // namespace

TEST(ConvertGpio, CsvAndVcdOfTheLines) {
    const auto bin = Path("gpio.bin");
    WriteGpio(bin);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToCSV(bin, "", FH_CSV_NONE));
    EXPECT_EQ(ReadText(Path("gpio.csv")), "sample,p,n\n0,0x01,0x10\n3,0x02,0x20\n");
    ASSERT_TRUE(c.convertToVCD(bin, ""));
    const auto vcd = ReadText(Path("gpio.vcd"));
    EXPECT_NE(vcd.find("#24000\nb00000010 p\nb00100000 n\n"), std::string::npos) << vcd;
    EXPECT_FALSE(c.convertToTDMS(bin, "")) << "a GPIO recording is no TDMS";
}

TEST(ConvertGpio, VcdIsForGpioRecordings) {
    const auto bin = Path("adc_vcd.bin");
    WriteRecording(bin);
    converter_lib::CConverter c;
    EXPECT_FALSE(c.convertToVCD(bin, ""));
}

// streaming_improvements 3.8: the output file, the channels, no overwrite without asking, the head
TEST(ConvertToCSV, TheOutputFileTheChannelsAndTheHead) {
    const auto bin = Path("opts.bin");
    WriteRecording(bin);
    const auto out = Path("opts_out.csv");
    std::filesystem::remove(out);
    converter_lib::CConverter c;
    c.setOutput(out);
    c.setChannels(0x2);
    c.setCSVHeader(true);
    c.setOverwrite(false);
    ASSERT_TRUE(c.convertToCSV(bin, "", FH_CSV_ADD_INDEX)) << c.lastError();
    EXPECT_EQ(ReadText(out), "index,ch2\n1,-1\n2,-2\n3,-\n4,-11\n5,-12\n");
    EXPECT_EQ(c.lastOutput(), out);
    EXPECT_EQ(c.lastChannels(), 1u);
    EXPECT_EQ(c.lastSamples(), 4u);
    EXPECT_FALSE(c.convertToCSV(bin, "", FH_CSV_ADD_INDEX)) << "the file is there now";
    EXPECT_NE(c.lastError().find("--force"), std::string::npos) << c.lastError();
    c.setOverwrite(true);
    EXPECT_TRUE(c.convertToCSV(bin, "", FH_CSV_ADD_INDEX));
}

TEST(ConvertToWAV, OnlyTheChannelsAskedFor) {
    const auto bin = Path("wav1.bin");
    WriteRecording(bin);
    converter_lib::CConverter c;
    c.setChannels(0x1);
    ASSERT_TRUE(c.convertToWAV(bin, "")) << c.lastError();
    std::ifstream in(Path("wav1.wav"), std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(bytes.size(), 44u + 5u * 2u) << "one channel";
    std::vector<int16_t> s(5);
    std::memcpy(s.data(), bytes.data() + 44, 10);
    EXPECT_EQ(s, (std::vector<int16_t>{1, 2, 0, 11, 12}));
}

TEST(ConvertToCSV, AFileThatIsNotThereSaysSo) {
    converter_lib::CConverter c;
    EXPECT_FALSE(c.convertToCSV(Path("missing.bin"), "", FH_CSV_ADD_INDEX));
    EXPECT_NE(c.lastError().find("Can't open"), std::string::npos) << c.lastError();
}
