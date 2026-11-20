/**
 * The GPIO input into a file: BIN read back by bin_lib, CSV rows where the lines change, VCD
 * changes in ps, the gaps of the recording and the sample limit.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <thread>
#include <cstdio>
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
    std::filesystem::remove(path + ".log.lost.txt");  // its lines are added to: a repeated run
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

// ---- limits and parts (streaming_improvements 4.11)

namespace {

// 10 buffers of the same words: 3 + 2 samples each, P and N as in kFirst then kSecond
auto WriteTen(CGpioFileWriter& w) -> std::vector<CGpioFileWriter::Result> {
    std::vector<CGpioFileWriter::Result> r;
    for (int i = 0; i < 10; i++) {
        std::vector<uint32_t> words = {inWord(0x01, 2, 0x10, 2), inWord(0x02, 1, 0x20, 1)};
        r.push_back(w.write(words.data(), words.size(), 0, 0, static_cast<uint32_t>(i)));
    }
    return r;
}

}  // namespace

TEST(GpioFileWriter, ABinInPartsHasAHeaderWithTheTimeOfItsFirstSample) {
    const auto base = Path("parts.bin");
    auto rec = Recording(CStreamSettings::GPIOFormat::BIN);
    rec.startTime = 1000000;
    rec.limits.splitTime = 5 * 2 * 8e-9;  // 2 buffers of 5 samples at 8 ns
    {
        CGpioFileWriter w;
        ASSERT_TRUE(w.open(base, rec));
        WriteTen(w);
        EXPECT_EQ(w.part(), 5);
        EXPECT_EQ(w.samples(), 50u) << "the time line of all the parts";
        w.close();
    }
    EXPECT_FALSE(std::filesystem::exists(base)) << "numbered from the first part";
    uint64_t words = 0;
    for (int k = 1; k <= 5; k++) {
        char n[16];
        snprintf(n, sizeof(n), "_%04d", k);
        const std::string p = Path(std::string("parts") + n + ".bin");
        FileReader r;
        ASSERT_TRUE(r.open(p)) << p << ": " << r.error();
        EXPECT_EQ(r.info().startTime, 1000000 + (k - 1) * 10 * 8) << "10 samples of 8 ns a part";
        BlockInfo b;
        std::vector<uint8_t> data;
        ASSERT_TRUE(r.nextBlock(b, data));
        EXPECT_EQ(b.position, 0u) << "a part from 0";
        words += data.size() / 4;
        while (r.nextBlock(b, data))
            words += data.size() / 4;
        EXPECT_TRUE(std::filesystem::exists(p + ".log.txt"));
    }
    EXPECT_EQ(words, 20u) << "every word in one of them";
}

TEST(GpioFileWriter, ACsvInPartsOfASizeStartsEveryPartWithItsHead) {
    const auto base = Path("parts.csv");
    auto rec = Recording(CStreamSettings::GPIOFormat::CSV);
    rec.limits.splitSize = 60;
    {
        CGpioFileWriter w;
        ASSERT_TRUE(w.open(base, rec));
        WriteTen(w);
        w.close();
    }
    int parts = 0, rows = 0;
    for (int k = 1;; k++) {
        char n[16];
        snprintf(n, sizeof(n), "_%04d", k);
        const std::string p = Path(std::string("parts") + n + ".csv");
        if (!std::filesystem::exists(p))
            break;
        parts++;
        const auto t = ReadText(p);
        EXPECT_EQ(t.rfind("sample,p,n\n", 0), 0u) << p;
        EXPECT_NE(t.find("\n0,0x01,0x10\n"), std::string::npos) << "the samples of a part from 0: " << p;
        rows += static_cast<int>(std::count(t.begin(), t.end(), '\n')) - 1;
    }
    EXPECT_GE(parts, 3);
    EXPECT_EQ(rows, 20) << "two changes a buffer, each in one part";
}

TEST(GpioFileWriter, TheSizeLimitKeepsOutTheBufferThatWouldGoOverIt) {
    const auto path = Path("size.bin");
    auto rec = Recording(CStreamSettings::GPIOFormat::BIN);
    uint64_t head = 0;
    {
        // the head of the file counts too
        const auto empty = Path("size_head.bin");
        CGpioFileWriter e;
        ASSERT_TRUE(e.open(empty, rec));
        e.close();
        head = std::filesystem::file_size(empty);
    }
    rec.limits.size = head + 3 * (48 + 8);  // the head and three blocks
    CGpioFileWriter w;
    ASSERT_TRUE(w.open(path, rec));
    const auto r = WriteTen(w);
    EXPECT_EQ(r[2], CGpioFileWriter::Result::OK);
    EXPECT_EQ(r[3], CGpioFileWriter::Result::LIMIT);
    EXPECT_EQ(r[9], CGpioFileWriter::Result::LIMIT);
    EXPECT_EQ(w.samples(), 15u);
    w.close();
    FileReader fr;
    ASSERT_TRUE(fr.open(path));
    EXPECT_EQ(fr.gaps(0).size(), 0u);
    BlockInfo b;
    std::vector<uint8_t> data;
    int blocks = 0;
    while (fr.nextBlock(b, data))
        blocks++;
    EXPECT_EQ(blocks, 3);
}

TEST(GpioFileWriter, TheTimeLimitEndsAtTheBufferThatReachesIt) {
    const auto path = Path("time.vcd");
    auto rec = Recording(CStreamSettings::GPIOFormat::VCD);
    rec.limits.time = 12 * 8e-9;  // 12 samples: the third buffer reaches it
    CGpioFileWriter w;
    ASSERT_TRUE(w.open(path, rec));
    const auto r = WriteTen(w);
    EXPECT_EQ(r[1], CGpioFileWriter::Result::OK);
    EXPECT_EQ(r[2], CGpioFileWriter::Result::LIMIT);
    EXPECT_EQ(r[3], CGpioFileWriter::Result::LIMIT);
    EXPECT_EQ(w.samples(), 15u);
}

// With RLE the lanes run on their own: a run of P that goes on in the next buffer goes on in the
// next part too, the lanes stay together
TEST(GpioFileWriter, ARunOverThePartBoundaryKeepsTheLanesTogether) {
    const auto base = Path("rle_parts.csv");
    auto rec = Recording(CStreamSettings::GPIOFormat::CSV);
    rec.limits.splitTime = 8e-9;  // a part a buffer
    // P 0x01 for 10, N 0x10 for 1; then P 0x02 for 1, N 0x20 for 10: 11 samples of each lane
    const std::vector<uint32_t> a = {inWord(0x01, 9, 0x10, 0)};
    const std::vector<uint32_t> b = {inWord(0x02, 0, 0x20, 9)};
    {
        CGpioFileWriter w;
        ASSERT_TRUE(w.open(base, rec));
        EXPECT_EQ(w.write(a.data(), a.size(), 0, 0, 0), CGpioFileWriter::Result::OK);
        EXPECT_EQ(w.write(b.data(), b.size(), 0, 0, 0), CGpioFileWriter::Result::OK);
        EXPECT_EQ(w.part(), 2);
        EXPECT_EQ(w.samples(), 11u);
        w.close();
    }
    EXPECT_EQ(ReadText(Path("rle_parts_0001.csv")), "sample,p,n\n0,0x01,0x10\n");
    EXPECT_EQ(ReadText(Path("rle_parts_0002.csv")), "sample,p,n\n0,0x01,0x20\n9,0x02,0x20\n") << "P 0x01 goes on for 9";
}

// a trigger after a wait past the time limit: the buffer stays out
TEST(GpioFileWriter, ABufferWhoseDataStartsPastTheTimeLimitStaysOut) {
    const auto path = Path("time_past.bin");
    auto rec = Recording(CStreamSettings::GPIOFormat::BIN);
    rec.limits.time = 12 * 8e-9;
    CGpioFileWriter w;
    ASSERT_TRUE(w.open(path, rec));
    EXPECT_EQ(w.write(kFirst.data(), kFirst.size(), 0, 0, 0), CGpioFileWriter::Result::OK);
    EXPECT_EQ(w.write(kSecond.data(), kSecond.size(), 0, 20, 1), CGpioFileWriter::Result::LIMIT);
    EXPECT_EQ(w.samples(), 3u);
    w.close();
    FileReader r;
    ASSERT_TRUE(r.open(path));
    BlockInfo blk;
    std::vector<uint8_t> data;
    int blocks = 0;
    while (r.nextBlock(blk, data))
        blocks++;
    EXPECT_EQ(blocks, 1);
}

// the size limit on a buffer that begins a part: no part of a head only
TEST(GpioFileWriter, NoPartOfAHeadOnlyIsBegunForABufferTheSizeLimitKeepsOut) {
    const auto base = Path("limit_parts.bin");
    auto rec = Recording(CStreamSettings::GPIOFormat::BIN);
    uint64_t head = 0;
    {
        const auto empty = Path("limit_parts_head.bin");
        CGpioFileWriter e;
        ASSERT_TRUE(e.open(empty, rec));
        e.close();
        head = std::filesystem::file_size(empty);
    }
    rec.limits.splitTime = 5 * 8e-9;  // a part a buffer
    rec.limits.size = 2 * (head + 48 + 8);  // two parts of a block
    CGpioFileWriter w;
    ASSERT_TRUE(w.open(base, rec));
    const auto r = WriteTen(w);
    EXPECT_EQ(r[1], CGpioFileWriter::Result::OK);
    EXPECT_EQ(r[2], CGpioFileWriter::Result::LIMIT);
    EXPECT_EQ(w.part(), 2);
    w.close();
    EXPECT_TRUE(std::filesystem::exists(Path("limit_parts_0002.bin")));
    EXPECT_FALSE(std::filesystem::exists(Path("limit_parts_0003.bin"))) << "no part for the buffer kept out";
    EXPECT_EQ(std::filesystem::file_size(Path("limit_parts_0001.bin")) + std::filesystem::file_size(Path("limit_parts_0002.bin")), rec.limits.size);
}

// a trigger that never comes: timeUp() once the time limit passed on the clock
TEST(GpioFileWriter, TheTimeLimitIsUpOnTheClockWhenNoBufferComes) {
    const auto path = Path("time_clock.bin");
    auto rec = Recording(CStreamSettings::GPIOFormat::BIN);
    rec.limits.time = 0.05;
    rec.bufferSeconds = 0.01;
    CGpioFileWriter w;
    w.setTimeMargin(0.05);
    ASSERT_TRUE(w.open(path, rec));
    EXPECT_FALSE(w.timeUp()) << "not yet";
    EXPECT_EQ(w.write(kFirst.data(), kFirst.size(), 0, 0, 0), CGpioFileWriter::Result::OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_TRUE(w.timeUp());
    EXPECT_FALSE(w.timeUp()) << "once";
    EXPECT_EQ(w.write(kSecond.data(), kSecond.size(), 0, 0, 1), CGpioFileWriter::Result::LIMIT) << "done";
    EXPECT_EQ(w.samples(), 3u);
    w.close();
    CGpioFileWriter none;
    ASSERT_TRUE(none.open(Path("time_none.bin"), Recording(CStreamSettings::GPIOFormat::BIN)));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(none.timeUp()) << "no time limit";
    none.close();
}
