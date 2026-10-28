/**
 * The TDMS API end to end: what SegmentBuilder writes, FileReader reads back, one segment per
 * buffer as a recorder writes them; and FileReader on files of the older File/Writer API, which
 * is how the board wrote TDMS so far.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "byte_utils.h"
#include "file.h"
#include "tdms.h"

using tdms_test::TempFile;
using TDMS::FileReader;
using TDMS::SegmentBuilder;
using TDMS::TDMSType;
using TDMS::TimeStamp;
using TDMS::Value;

namespace {

auto OpenString(FileReader& f, const std::string& bytes) -> bool {
    return f.open(std::make_unique<std::stringstream>(bytes, std::ios::in | std::ios::binary));
}

}  // namespace

TEST(ApiRoundTrip, ARecordingOfSeveralBuffersWithItsMetadata) {
    std::ostringstream file(std::ios::binary);
    std::vector<int16_t> ch1All;
    std::vector<int16_t> ch2All;
    for (int buffer = 0; buffer < 5; buffer++) {
        SegmentBuilder s;
        if (buffer == 0) {
            s.setProperty("device", "STEMlab 125-14");
            s.setProperty("start", TimeStamp::fromUnix(1700000000, 250));
            s.setGroupProperty("Group", "decimation", uint32_t(8));
            s.setChannelProperty("Group", "ch1", "wf_increment", 64e-9);
            s.setChannelProperty("Group", "ch1", "unit_string", "V");
        }
        // per buffer metadata: the latest value wins
        s.setGroupProperty("Group", "buffers", uint32_t(buffer + 1));
        std::vector<int16_t> ch1(100);
        std::vector<int16_t> ch2(100);
        for (int i = 0; i < 100; i++) {
            ch1[i] = static_cast<int16_t>(buffer * 100 + i);
            ch2[i] = static_cast<int16_t>(-ch1[i]);
        }
        s.copySamples("Group", "ch1", ch1);
        s.copySamples("Group", "ch2", ch2);
        ch1All.insert(ch1All.end(), ch1.begin(), ch1.end());
        ch2All.insert(ch2All.end(), ch2.begin(), ch2.end());
        s.write(file);
    }

    FileReader f;
    ASSERT_TRUE(OpenString(f, file.str())) << f.error();
    EXPECT_EQ(f.properties().at("device"), Value("STEMlab 125-14"));
    EXPECT_EQ(TDMS::getProperty<TimeStamp>(f.properties(), "start")->unixNs(), 1700000000LL * 1000000000LL + 250);
    EXPECT_EQ(TDMS::getProperty<uint32_t>(f.groupProperties("Group"), "decimation"), 8u);
    EXPECT_EQ(TDMS::getProperty<uint32_t>(f.groupProperties("Group"), "buffers"), 5u);
    ASSERT_EQ(f.channels("Group").size(), 2u);
    EXPECT_EQ(f.channels()[0].name, "ch1");
    EXPECT_EQ(f.channels()[1].name, "ch2");
    EXPECT_EQ(TDMS::getProperty<std::string>(f.channel("Group", "ch1")->properties, "unit_string"), std::string("V"));
    EXPECT_EQ(f.channel("Group", "ch2")->count, 500u);
    EXPECT_EQ(f.readAll<int16_t>("Group", "ch1"), ch1All);
    EXPECT_EQ(f.readAll<int16_t>("Group", "ch2"), ch2All);
    EXPECT_EQ(f.read<int16_t>("Group", "ch2", 99, 2), (std::vector<int16_t>{-99, -100})) << "across two segments";
}

TEST(ApiRoundTrip, SamplesAreReadStraightIntoTheCallersMemory) {
    std::ostringstream file(std::ios::binary);
    for (int seg = 0; seg < 3; seg++) {
        SegmentBuilder s;
        const std::vector<int16_t> v{int16_t(seg * 10), int16_t(seg * 10 + 1)};
        s.copySamples("G", "a", v);
        s.write(file);
    }
    FileReader f;
    ASSERT_TRUE(OpenString(f, file.str())) << f.error();
    int16_t block[5] = {-1, -1, -1, -1, -1};
    EXPECT_EQ(f.read("G", "a", 1, 4, block), 4u);
    EXPECT_EQ(std::vector<int16_t>(block, block + 5), (std::vector<int16_t>{1, 10, 11, 20, -1})) << "only the samples read are written";
    EXPECT_EQ(f.read("G", "a", 5, 4, block), 1u);
    EXPECT_EQ(block[0], 21);
    EXPECT_EQ(f.read("G", "a", 6, 4, block), 0u);
    EXPECT_EQ(f.read("G", "missing", 0, 4, block), 0u);
}

TEST(ApiRoundTrip, AChannelMayBeMissingFromSomeSegments) {
    std::ostringstream file(std::ios::binary);
    const std::vector<int32_t> a{1, 2};
    const std::vector<int32_t> b{9};
    for (int s = 0; s < 3; s++) {
        SegmentBuilder seg;
        seg.copySamples("G", "a", a);
        if (s == 1)
            seg.copySamples("G", "b", b);
        seg.write(file);
    }
    FileReader f;
    ASSERT_TRUE(OpenString(f, file.str())) << f.error();
    EXPECT_EQ(f.readAll<int32_t>("G", "a"), (std::vector<int32_t>{1, 2, 1, 2, 1, 2}));
    EXPECT_EQ(f.readAll<int32_t>("G", "b"), (std::vector<int32_t>{9}));
}

TEST(ApiRoundTrip, EveryPropertyTypeComesBack) {
    SegmentBuilder s;
    const std::vector<std::pair<std::string, Value>> values{
        {"i8", Value(int8_t(-8))},       {"i16", Value(int16_t(-16))},      {"i32", Value(int32_t(-32))},     {"i64", Value(int64_t(-64))},
        {"u8", Value(uint8_t(8))},       {"u16", Value(uint16_t(16))},      {"u32", Value(uint32_t(32))},     {"u64", Value(uint64_t(64))},
        {"f32", Value(1.5f)},            {"f64", Value(-2.25)},             {"bool", Value(false)},           {"text", Value("π and 'quotes'")},
        {"empty", Value(std::string())}, {"ts", Value(TimeStamp::fromUnixNs(-5))},
    };
    for (const auto& [k, v] : values)
        s.setChannelProperty("G", "c", k, v);
    FileReader f;
    ASSERT_TRUE(OpenString(f, s.bytes())) << f.error();
    const auto* c = f.channel("G", "c");
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->type, TDMSType::Empty) << "properties only";
    EXPECT_EQ(c->count, 0u);
    for (const auto& [k, v] : values) {
        ASSERT_EQ(c->properties.count(k), 1u) << k;
        EXPECT_EQ(c->properties.at(k), v) << k;
    }
}

TEST(ApiRoundTrip, EveryDataTypeComesBack) {
    SegmentBuilder s;
    const std::vector<int8_t> i8{-1, 2};
    const std::vector<uint16_t> u16{65535, 1};
    const std::vector<int64_t> i64{-1, 1LL << 40};
    const std::vector<float> f32{0.5f, -1.0f};
    const std::vector<double> f64{1e300, -0.0};
    s.copySamples("G", "i8", i8);
    s.copySamples("G", "u16", u16);
    s.copySamples("G", "i64", i64);
    s.copySamples("G", "f32", f32);
    s.copySamples("G", "f64", f64);
    FileReader f;
    ASSERT_TRUE(OpenString(f, s.bytes())) << f.error();
    EXPECT_EQ(f.readAll<int8_t>("G", "i8"), i8);
    EXPECT_EQ(f.readAll<uint16_t>("G", "u16"), u16);
    EXPECT_EQ(f.readAll<int64_t>("G", "i64"), i64);
    EXPECT_EQ(f.readAll<float>("G", "f32"), f32);
    EXPECT_EQ(f.readAll<double>("G", "f64"), f64);
    EXPECT_EQ(f.channel("G", "f64")->type, TDMSType::DoubleFloat);
}

// The layout the board wrote with File/Writer: per buffer a segment of the group, a time column
// and the channels, no new object list. A player picks the columns it wants.
TEST(ApiRoundTrip, ColumnsOfAnOldBoardFileArePickedByName) {
    TempFile tmp("board.tdms");
    for (int seg = 0; seg < 3; seg++) {
        TDMS::WriterSegment segment;
        std::vector<std::shared_ptr<TDMS::Metadata>> nodes;
        auto root = segment.GenerateRoot();
        root->TableOfContents.HasMetaData = true;
        root->TableOfContents.HasRawData = true;
        nodes.push_back(root);
        nodes.push_back(segment.GenerateGroup("Group"));
        auto time = segment.GenerateChannel("Group", "time");
        nodes.push_back(time);
        std::shared_ptr<std::uint8_t[]> t(new std::uint8_t[4 * 8]);
        for (int i = 0; i < 4; i++)
            reinterpret_cast<int64_t*>(t.get())[i] = seg * 1000 + i;
        segment.AddRaw(time, TDMSType::Integer64, 4, t);
        for (const char* name : {"ch1", "ch2"}) {
            auto c = segment.GenerateChannel("Group", name);
            nodes.push_back(c);
            std::shared_ptr<std::uint8_t[]> d(new std::uint8_t[4 * 2]);
            for (int i = 0; i < 4; i++)
                reinterpret_cast<int16_t*>(d.get())[i] = static_cast<int16_t>((name[2] == '1' ? 1 : -1) * (seg * 10 + i));
            segment.AddRaw(c, TDMSType::Integer16, 4, d);
        }
        segment.LoadMetadata(nodes);
        TDMS::File().WriteFile(tmp.path(), segment, seg > 0);
    }

    FileReader f;
    ASSERT_TRUE(f.open(tmp.path())) << f.error();
    ASSERT_EQ(f.channels().size(), 3u);
    EXPECT_EQ(f.channels()[0].name, "time");
    EXPECT_EQ(f.readAll<int16_t>("Group", "ch2"), (std::vector<int16_t>{0, -1, -2, -3, -10, -11, -12, -13, -20, -21, -22, -23}));
    EXPECT_EQ(f.read<int64_t>("Group", "time", 4, 1), (std::vector<int64_t>{1000}));
}

TEST(ApiRoundTrip, AFileOfManySegmentsOpensQuickly) {
    std::ostringstream file(std::ios::binary);
    const std::vector<int16_t> samples(64, 1);
    for (int seg = 0; seg < 20000; seg++) {
        SegmentBuilder s;
        s.copySamples("Group", "ch1", samples);
        s.copySamples("Group", "ch2", samples);
        s.write(file);
    }
    const auto start = std::chrono::steady_clock::now();
    FileReader f;
    ASSERT_TRUE(OpenString(f, file.str())) << f.error();
    EXPECT_EQ(f.channel("Group", "ch2")->count, 20000u * 64u);
    EXPECT_EQ(f.read<int16_t>("Group", "ch1", 20000u * 64u - 1, 5).size(), 1u);
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(5));
}

TEST(ApiRoundTrip, CloseForgetsTheFile) {
    SegmentBuilder s;
    s.setProperty("a", 1.0);
    FileReader f;
    ASSERT_TRUE(OpenString(f, s.bytes()));
    f.close();
    EXPECT_FALSE(f.isOpen());
    EXPECT_TRUE(f.properties().empty());
    EXPECT_TRUE(f.channels().empty());
}
