/**
 * The GPIO input into a file: BIN read back by bin_lib, CSV rows where the lines change, VCD
 * changes in ps, the gaps of the recording and the sample limit.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "bin_lib/bin_file.h"
#include "bin_lib/gpio_words.h"
#include "writer_lib/gpio_file_writer.h"

using namespace bin_lib;

namespace {

auto Path(const std::string& name) -> std::string {
    auto dir = std::filesystem::temp_directory_path() / ("gpio_writer_" + std::to_string(::getpid()));
    std::filesystem::create_directories(dir);
    return (dir / name).string();
}

auto ReadText(const std::string& path) -> std::string {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

auto Recording(CStreamSettings::GPIOFormat format, bool rle = true) -> GpioRecording {
    GpioRecording r;
    r.format = format;
    r.rle = rle;
    r.clock = 125000000;
    r.decimation = 1;  // 8000 ps a sample
    r.startTime = 42;
    return r;
}

// Two buffers: P 0x01 for 3 then 0x02 for 2 (N alike with 0x10, 0x20); the second after 4 lost
const std::vector<uint32_t> kFirst = {inWord(0x01, 2, 0x10, 2)};
const std::vector<uint32_t> kSecond = {inWord(0x02, 1, 0x20, 1)};

}  // namespace

TEST(GpioFileWriter, BinKeepsTheWordsAndTheirTimeline) {
    const auto path = Path("in.bin");
    {
        CGpioFileWriter w;
        ASSERT_TRUE(w.open(path, Recording(CStreamSettings::GPIOFormat::BIN)));
        EXPECT_EQ(w.write(kFirst.data(), kFirst.size(), 0, 0, 1), CGpioFileWriter::Result::OK);
        EXPECT_EQ(w.write(kSecond.data(), kSecond.size(), 4, 0, 2), CGpioFileWriter::Result::OK);
        EXPECT_EQ(w.samples(), 9u);
        w.close();
    }
    FileReader r;
    ASSERT_TRUE(r.open(path)) << r.error();
    EXPECT_EQ(r.info().source, Source::GPIO_IN);
    ASSERT_EQ(r.info().channels.size(), 1u);
    EXPECT_EQ(r.info().channels[0].type, SampleType::GPIO_IN_WORD);
    EXPECT_EQ(r.info().channels[0].flags, CHANNEL_FLAG_RLE);
    EXPECT_EQ(r.info().startTime, 42);
    BlockInfo b;
    std::vector<uint8_t> data;
    ASSERT_TRUE(r.nextBlock(b, data));
    EXPECT_EQ(b.samples, 3u);
    EXPECT_EQ(reinterpret_cast<uint32_t*>(data.data())[0], kFirst[0]);
    ASSERT_TRUE(r.nextBlock(b, data));
    EXPECT_EQ(b.lost, 4u);
    EXPECT_EQ(b.position, 7u);
    EXPECT_EQ(b.triggerCount, 2u);
}

TEST(GpioFileWriter, CsvHasARowWhereTheLinesChange) {
    const auto path = Path("in.csv");
    {
        CGpioFileWriter w;
        ASSERT_TRUE(w.open(path, Recording(CStreamSettings::GPIOFormat::CSV)));
        w.write(kFirst.data(), kFirst.size(), 0, 0, 0);
        const std::vector<uint32_t> same = {inWord(0x01, 0, 0x10, 0)};  // the same levels go on
        w.write(same.data(), same.size(), 0, 0, 0);
        w.write(kSecond.data(), kSecond.size(), 4, 0, 0);
        w.close();
    }
    EXPECT_EQ(ReadText(path), "sample,p,n\n0,0x01,0x10\n4,-,-\n8,0x02,0x20\n");
}

TEST(GpioFileWriter, VcdChangesInPicoseconds) {
    const auto path = Path("in.vcd");
    {
        CGpioFileWriter w;
        ASSERT_TRUE(w.open(path, Recording(CStreamSettings::GPIOFormat::VCD)));
        // P changes at 3, N stays
        const std::vector<uint32_t> words = {inWord(0x01, 2, 0x10, 4), inWord(0x03, 1, 0x10, 0)};
        w.write(words.data(), words.size(), 0, 0, 0);
        w.write(kSecond.data(), kSecond.size(), 1, 0, 0);
        w.close();
    }
    const auto vcd = ReadText(path);
    EXPECT_NE(vcd.find("$timescale 1 ps $end"), std::string::npos);
    EXPECT_NE(vcd.find("$var wire 8 p p[7:0] $end"), std::string::npos);
    const auto body = vcd.substr(vcd.find("$enddefinitions $end\n") + 21);
    EXPECT_EQ(body,
              "#0\nb00000001 p\nb00010000 n\n"
              "#24000\nb00000011 p\n"
              "#40000\nbxxxxxxxx p\nbxxxxxxxx n\n"
              "#48000\nb00000010 p\nb00100000 n\n");
}

TEST(GpioFileWriter, TheLimitEndsTheFileAtTheBufferThatReachesIt) {
    const auto path = Path("limit.bin");
    auto rec = Recording(CStreamSettings::GPIOFormat::BIN);
    rec.samplesLimit = 4;
    CGpioFileWriter w;
    ASSERT_TRUE(w.open(path, rec));
    EXPECT_EQ(w.write(kFirst.data(), kFirst.size(), 0, 0, 0), CGpioFileWriter::Result::OK);
    EXPECT_EQ(w.write(kSecond.data(), kSecond.size(), 0, 0, 0), CGpioFileWriter::Result::LIMIT);
    EXPECT_EQ(w.write(kFirst.data(), kFirst.size(), 0, 0, 0), CGpioFileWriter::Result::LIMIT) << "nothing more";
    w.close();
    FileReader r;
    ASSERT_TRUE(r.open(path));
    BlockInfo b;
    std::vector<uint8_t> data;
    int blocks = 0;
    while (r.nextBlock(b, data))
        blocks++;
    EXPECT_EQ(blocks, 2);
    // the report tells the limit and how far past it the last buffer went
    const auto report = ReadText(path + ".log.txt");
    EXPECT_NE(report.find("Samples limit:\t4\n"), std::string::npos) << report;
    EXPECT_NE(report.find("Time line recorded:\t5 Samples\n"), std::string::npos) << report;
    EXPECT_NE(report.find("the last buffer ends 1 Samples after it"), std::string::npos) << report;
    const auto lines = ReadText(path + ".log.lost.txt");
    EXPECT_EQ(std::count(lines.begin(), lines.end(), '\n'), 3) << "the legend and a line per buffer";
}

TEST(GpioFileWriter, TheReportSaysWhenTheLimitIsNotReached) {
    const auto path = Path("short.csv");
    auto rec = Recording(CStreamSettings::GPIOFormat::CSV);
    rec.samplesLimit = 100;
    {
        CGpioFileWriter w;
        ASSERT_TRUE(w.open(path, rec));
        EXPECT_EQ(w.write(kFirst.data(), kFirst.size(), 0, 0, 0), CGpioFileWriter::Result::OK);
    }
    const auto report = ReadText(path + ".log.txt");
    EXPECT_NE(report.find("Samples limit:\t100\n"), std::string::npos) << report;
    EXPECT_NE(report.find("The limit is not reached"), std::string::npos) << report;
}

TEST(GpioFileWriter, AFileThatCannotBeMadeIsAnError) {
    CGpioFileWriter w;
    EXPECT_FALSE(w.open("/no/such/dir/in.bin", Recording(CStreamSettings::GPIOFormat::BIN)));
    EXPECT_EQ(w.write(kFirst.data(), 1, 0, 0, 0), CGpioFileWriter::Result::ERROR);
}
