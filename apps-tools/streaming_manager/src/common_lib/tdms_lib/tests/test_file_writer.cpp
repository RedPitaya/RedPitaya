/**
 * TDMS::FileWriter and the waveform properties: segments written into a file one after another,
 * appended to an existing file, and a file cut off in its last segment still read up to there.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "byte_utils.h"
#include "tdms.h"

using tdms_test::TempFile;
using TDMS::FileReader;
using TDMS::FileWriter;
using TDMS::SegmentBuilder;
using TDMS::TimeStamp;

namespace {

auto Segment(int n) -> SegmentBuilder {
    SegmentBuilder s;
    s.setGroupProperty("G", "segment", int32_t(n));
    auto samples = std::make_shared<std::vector<int16_t>>();
    for (int i = 0; i < 4; i++)
        samples->push_back(static_cast<int16_t>(n * 10 + i));
    s.addSamples("G", "a", samples);
    return s;
}

}  // namespace

TEST(FileWriter, SegmentsGoIntoTheFileOneAfterAnother) {
    TempFile tmp("w.tdms");
    FileWriter w;
    ASSERT_TRUE(w.open(tmp.path())) << w.error();
    std::uint64_t expected = 0;
    for (int n = 0; n < 3; n++) {
        auto s = Segment(n);
        expected += s.size();
        ASSERT_TRUE(w.write(s)) << w.error();
    }
    EXPECT_EQ(w.size(), expected);
    ASSERT_TRUE(w.close()) << w.error();
    EXPECT_EQ(std::filesystem::file_size(tmp.path()), expected);

    FileReader f;
    ASSERT_TRUE(f.open(tmp.path())) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{0, 1, 2, 3, 10, 11, 12, 13, 20, 21, 22, 23}));
    EXPECT_EQ(TDMS::getProperty<int32_t>(f.groupProperties("G"), "segment"), 2);
    EXPECT_FALSE(f.truncated());
}

TEST(FileWriter, AppendContinuesAFileAndANewFileReplacesIt) {
    TempFile tmp("append.tdms");
    {
        FileWriter w;
        ASSERT_TRUE(w.open(tmp.path()));
        auto s = Segment(0);
        w.write(s);
    }
    FileWriter w;
    ASSERT_TRUE(w.open(tmp.path(), true));
    const auto before = w.size();
    EXPECT_GT(before, 0u);
    auto s = Segment(1);
    w.write(s);
    EXPECT_EQ(w.size(), before + s.size());
    w.close();
    FileReader f;
    ASSERT_TRUE(f.open(tmp.path()));
    EXPECT_EQ(f.channel("G", "a")->count, 8u);

    ASSERT_TRUE(w.open(tmp.path()));
    auto t = Segment(5);
    w.write(t);
    w.close();
    ASSERT_TRUE(f.open(tmp.path()));
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{50, 51, 52, 53}));
}

TEST(FileWriter, WritingWithoutAFileIsAnError) {
    FileWriter w;
    auto s = Segment(0);
    EXPECT_FALSE(w.write(s));
    EXPECT_FALSE(w.open("/nonexistent/dir/x.tdms"));
    EXPECT_NE(w.error().find("cannot open"), std::string::npos);
}

// A recording stopped by a full disk or a power loss: every cut inside the last segment leaves
// the segments before it readable
TEST(FileWriter, AFileCutOffInItsLastSegmentReadsUpToThere) {
    TempFile tmp("cut.tdms");
    std::string bytes;
    for (int n = 0; n < 3; n++)
        bytes += Segment(n).bytes();
    const std::size_t lastStart = bytes.size() - Segment(2).bytes().size();
    for (std::size_t cut = lastStart + 1; cut < bytes.size(); cut++) {
        {
            std::ofstream out(tmp.path(), std::ios::binary | std::ios::trunc);
            out.write(bytes.data(), static_cast<std::streamsize>(cut));
        }
        FileReader f;
        ASSERT_TRUE(f.open(tmp.path())) << "cut at " << cut << ": " << f.error();
        const auto count = f.channel("G", "a")->count;
        EXPECT_GE(count, 8u) << "cut at " << cut;
        EXPECT_LE(count, 12u);
        auto a = f.readAll<int16_t>("G", "a");
        ASSERT_EQ(a.size(), count);
        EXPECT_EQ(a[7], 13);
    }
}

TEST(Waveform, TheTimePropertiesComeBack) {
    SegmentBuilder s;
    TDMS::setWaveform(s, "G", "ch1", TDMS::Waveform{TimeStamp::fromUnix(1700000000, 5), 8e-9, 1e-6});
    FileReader f;
    ASSERT_TRUE(f.open(std::make_unique<std::stringstream>(s.bytes(), std::ios::in | std::ios::binary)));
    auto w = TDMS::waveformOf(*f.channel("G", "ch1"));
    ASSERT_TRUE(w);
    EXPECT_EQ(w->start, TimeStamp::fromUnix(1700000000, 5));
    EXPECT_EQ(w->increment, 8e-9);
    EXPECT_EQ(w->offset, 1e-6);
    TDMS::ChannelInfo bare;
    EXPECT_FALSE(TDMS::waveformOf(bare));
}
