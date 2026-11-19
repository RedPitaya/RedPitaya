/**
 * The BIN stream file: what is written is read back block by block and sample by sample, with
 * and without the gaps of the recording; a file cut short or of another kind is handled.
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include "bin_lib/bin_file.h"

using namespace bin_lib;

namespace {

class BinFile : public ::testing::Test {
   protected:
    void SetUp() override {
        m_path = (std::filesystem::temp_directory_path() / ("bin_file_" + std::to_string(::getpid()) + ".bin")).string();
    }
    void TearDown() override { std::filesystem::remove(m_path); }

    auto write(const std::vector<uint8_t>& bytes) -> void {
        std::ofstream f(m_path, std::ios::binary | std::ios::trunc);
        f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    // Two int16 channels, three buffers: channel i of buffer b holds b * 100 + i * 1000 + k.
    // Buffer 1 comes after 5 lost samples, buffer 2 after 3 samples of trigger wait.
    auto twoChannels() -> std::vector<uint8_t> {
        FileInfo info;
        info.source = Source::ADC;
        info.flags = FILE_FLAG_TRIGGER_MODE | FILE_FLAG_WAIT_COUNTER;
        info.clock = 125000000;
        info.decimation = 8;
        info.startTime = 1234567890123456789;
        info.channels = {{1, SampleType::INT16, 0, 14}, {2, SampleType::INT16, 0, 14}};
        auto out = fileHeader(info);
        Timeline line;
        const uint64_t lost[3] = {0, 5, 0};
        const uint64_t wait[3] = {0, 0, 3};
        for (int b = 0; b < 3; b++) {
            for (uint8_t ch = 0; ch < 2; ch++) {
                std::vector<int16_t> s(kSamples);
                for (int k = 0; k < kSamples; k++)
                    s[k] = static_cast<int16_t>(b * 100 + ch * 1000 + k);
                BlockInfo block;
                block.channel = ch;
                block.triggerCount = b;
                block.samples = kSamples;
                block.lost = lost[b];
                block.wait = wait[b];
                block.position = line.next(ch, lost[b], wait[b], kSamples);
                appendBlock(out, block, s.data(), kSamples * 2);
            }
        }
        return out;
    }

    static constexpr int kSamples = 10;
    std::string m_path;
};

}  // namespace

TEST_F(BinFile, TheHeaderIsReadBack) {
    write(twoChannels());
    FileReader r;
    ASSERT_TRUE(r.open(m_path)) << r.error();
    const auto& info = r.info();
    EXPECT_EQ(info.source, Source::ADC);
    EXPECT_EQ(info.flags, FILE_FLAG_TRIGGER_MODE | FILE_FLAG_WAIT_COUNTER);
    EXPECT_EQ(info.clock, 125000000u);
    EXPECT_EQ(info.decimation, 8u);
    EXPECT_EQ(info.startTime, 1234567890123456789);
    ASSERT_EQ(info.channels.size(), 2u);
    EXPECT_EQ(info.channels[1].id, 2);
    EXPECT_EQ(info.channels[1].type, SampleType::INT16);
    EXPECT_EQ(info.channels[1].adcBits, 14);
}

TEST_F(BinFile, TheBlocksComeBackInOrderWithTheirTimeline) {
    write(twoChannels());
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    BlockInfo b;
    std::vector<uint8_t> data;
    std::vector<uint64_t> positions;
    int blocks = 0;
    while (r.nextBlock(b, data)) {
        EXPECT_EQ(b.channel, blocks % 2);
        EXPECT_EQ(data.size(), kSamples * 2u);
        EXPECT_EQ(reinterpret_cast<int16_t*>(data.data())[0], (blocks / 2) * 100 + b.channel * 1000);
        if (b.channel == 0)
            positions.push_back(b.position);
        blocks++;
    }
    EXPECT_EQ(blocks, 6);
    EXPECT_EQ(positions, (std::vector<uint64_t>{0, 15, 28})) << "after 5 lost, then after 3 waited";
    r.rewind();
    ASSERT_TRUE(r.nextBlock(b, data));
    EXPECT_EQ(b.position, 0u);
}

TEST_F(BinFile, WithoutGapsTheSamplesFollowEachOther) {
    write(twoChannels());
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    EXPECT_EQ(r.samples(0, false), 30u);
    std::vector<int16_t> s(30);
    ASSERT_EQ(r.read(1, 0, 30, s.data(), false), 30u);
    for (int i = 0; i < 30; i++)
        EXPECT_EQ(s[i], (i / 10) * 100 + 1000 + i % 10) << i;
    // from the middle of a block across the next
    std::vector<int16_t> part(8);
    ASSERT_EQ(r.read(0, 6, 8, part.data(), false), 8u);
    EXPECT_EQ(part[0], 6);
    EXPECT_EQ(part[4], 100);
}

TEST_F(BinFile, WithGapsTheRecordingKeepsItsTime) {
    write(twoChannels());
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    EXPECT_EQ(r.samples(0, true), 38u) << "30 samples, 5 lost, 3 waited";
    std::vector<int16_t> s(38, -1);
    ASSERT_EQ(r.read(0, 0, 38, s.data(), true), 38u);
    for (int i = 0; i < 10; i++)
        EXPECT_EQ(s[i], i);
    for (int i = 10; i < 15; i++)
        EXPECT_EQ(s[i], 0) << "lost " << i;
    for (int i = 15; i < 25; i++)
        EXPECT_EQ(s[i], 100 + i - 15);
    for (int i = 25; i < 28; i++)
        EXPECT_EQ(s[i], 0) << "waited " << i;
    for (int i = 28; i < 38; i++)
        EXPECT_EQ(s[i], 200 + i - 28);
    // a read that starts inside a gap
    std::vector<int16_t> part(4, -1);
    ASSERT_EQ(r.read(0, 13, 4, part.data(), true), 4u);
    EXPECT_EQ(part, (std::vector<int16_t>{0, 0, 100, 101}));
}

TEST_F(BinFile, TheGapsAreListedWithTheirLostAndWaitedSamples) {
    write(twoChannels());
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    const auto g = r.gaps(0);
    ASSERT_EQ(g.size(), 2u);
    EXPECT_EQ(g[0].at, 10u);
    EXPECT_EQ(g[0].lost, 5u);
    EXPECT_EQ(g[0].wait, 0u);
    EXPECT_EQ(g[1].at, 25u);
    EXPECT_EQ(g[1].lost, 0u);
    EXPECT_EQ(g[1].wait, 3u);
    EXPECT_TRUE(r.gaps(7).empty()) << "not a channel";
}

TEST_F(BinFile, TheHeaderAloneIsReadWithoutTheBlocks) {
    write(twoChannels());
    FileReader r;
    ASSERT_TRUE(r.openHeader(m_path));
    EXPECT_EQ(r.info().channels.size(), 2u);
    EXPECT_EQ(r.info().decimation, 8u);
    EXPECT_EQ(r.samples(0, true), 0u) << "the blocks are not indexed";
    // a full open after it indexes again
    ASSERT_TRUE(r.open(m_path));
    EXPECT_EQ(r.samples(0, true), 38u);
}

TEST_F(BinFile, TheProgressOfTheOpenStopsIt) {
    write(twoChannels());
    FileReader r;
    double seen = -1;
    r.setProgress([&seen](double p) {
        seen = p;
        return false;
    });
    EXPECT_FALSE(r.open(m_path));
    EXPECT_EQ(r.error(), "stopped");
    EXPECT_GT(seen, 0.0);
    EXPECT_LE(seen, 1.0);
    r.setProgress({});
    EXPECT_TRUE(r.open(m_path)) << "without the callback it opens";
}

TEST_F(BinFile, ReadingPastTheEndGivesWhatThereIs) {
    write(twoChannels());
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    std::vector<int16_t> s(10);
    EXPECT_EQ(r.read(0, 25, 10, s.data(), false), 5u);
    EXPECT_EQ(r.read(0, 30, 10, s.data(), false), 0u);
}

TEST_F(BinFile, ARecordingCutShortEndsWithItsLastWholeBlock) {
    auto bytes = twoChannels();
    bytes.resize(bytes.size() - 7);
    write(bytes);
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    BlockInfo b;
    std::vector<uint8_t> data;
    int blocks = 0;
    while (r.nextBlock(b, data))
        blocks++;
    EXPECT_EQ(blocks, 5);
    EXPECT_EQ(r.samples(0, false), 30u);
    EXPECT_EQ(r.samples(1, false), 20u);
}

TEST_F(BinFile, BlocksAreAlignedTo8Bytes) {
    FileInfo info;
    info.channels = {{1, SampleType::INT8, 0, 0}};
    auto out = fileHeader(info);
    EXPECT_EQ(out.size() % 8, 0u);
    const uint8_t s[3] = {1, 2, 3};
    BlockInfo b;
    b.samples = 3;
    appendBlock(out, b, s, 3);
    EXPECT_EQ(out.size() % 8, 0u);
    appendBlock(out, b, s, 3);
    write(out);
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    std::vector<uint8_t> all(6);
    ASSERT_EQ(r.read(0, 0, 6, all.data(), false), 6u);
    EXPECT_EQ(all, (std::vector<uint8_t>{1, 2, 3, 1, 2, 3}));
}

TEST_F(BinFile, GpioWordsWithRLEAreBlocksOnly) {
    FileInfo info;
    info.source = Source::GPIO_IN;
    info.channels = {{1, SampleType::GPIO_IN_WORD, CHANNEL_FLAG_RLE, 0}};
    auto out = fileHeader(info);
    const uint32_t words[2] = {0x00050003, 0x01020304};
    BlockInfo b;
    b.samples = 1000;  // what the run lengths stand for
    appendBlock(out, b, words, sizeof(words));
    write(out);
    FileReader r;
    ASSERT_TRUE(r.open(m_path));
    EXPECT_EQ(r.samples(0, false), 0u) << "no positions inside RLE words";
    std::vector<uint8_t> data;
    ASSERT_TRUE(r.nextBlock(b, data));
    EXPECT_EQ(b.samples, 1000u);
    ASSERT_EQ(data.size(), 8u);
    EXPECT_EQ(reinterpret_cast<uint32_t*>(data.data())[1], 0x01020304u);
}

TEST_F(BinFile, AnotherFileIsRefused) {
    write({'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E'});
    FileReader r;
    EXPECT_FALSE(r.open(m_path));
    EXPECT_NE(r.error().find("not a BIN"), std::string::npos);

    auto bytes = twoChannels();
    bytes[8] = 2;  // version 2
    write(bytes);
    EXPECT_FALSE(r.open(m_path));
    EXPECT_NE(r.error().find("version 2"), std::string::npos);

    EXPECT_FALSE(r.open(m_path + ".none"));
}
