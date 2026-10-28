/**
 * TDMS::FileReader on files stated byte by byte from the TDMS 2.0 specification
 * (spec_builder.h), covering what other writers put in a file: incremental metadata, the
 * "same index as before" and "no data" raw data indexes, several chunks in one segment,
 * interleaved data, a recording cut off in the middle, and files it has to refuse.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "byte_utils.h"
#include "spec_builder.h"
#include "tdms.h"

using namespace tdms_test;
using TDMS::FileReader;
using TDMS::TDMSType;
using TDMS::Value;

namespace {

constexpr std::uint32_t kSameRawIndex = 0;

auto Open(FileReader& f, const Bytes& bytes) -> bool {
    return f.open(std::make_unique<std::stringstream>(std::string(bytes.begin(), bytes.end()), std::ios::in | std::ios::binary));
}

// A segment of the given flags from its metadata and raw bytes
auto Seg(std::int32_t toc, const Builder& meta, const Builder& raw) -> Builder {
    Builder b;
    b.LeadIn(toc, static_cast<std::int64_t>(meta.size() + raw.size()), static_cast<std::int64_t>(meta.size()));
    b.Raw(meta.bytes()).Raw(raw.bytes());
    return b;
}

auto Cat(std::initializer_list<Builder> parts) -> Bytes {
    Builder b;
    for (const auto& p : parts)
        b.Raw(p.bytes());
    return b.bytes();
}

constexpr std::int32_t kFirst = kTocMetaData | kTocNewObjList | kTocRawData;

// Segment 1 of the incremental tests: a (I16, 2 values) and b (I32, 1 value)
auto FirstAB() -> Builder {
    Builder meta;
    meta.U32(3).ObjectNoRaw("/'G'", 0).ObjectWithRaw("/'G'/'a'", kTypeI16, 2, 0).ObjectWithRaw("/'G'/'b'", kTypeI32, 1, 0);
    Builder raw;
    raw.I16(1).I16(2).I32(100);
    return Seg(kFirst, meta, raw);
}

}  // namespace

TEST(FileReader, NotATdmsFileIsRefused) {
    FileReader f;
    EXPECT_FALSE(Open(f, ToBytes("RIFF....WAVEfmt this is not a tdms file")));
    EXPECT_NE(f.error().find("not a TDMS file"), std::string::npos) << f.error();
    EXPECT_FALSE(f.isOpen());
    EXPECT_FALSE(Open(f, Bytes{}));
    EXPECT_FALSE(f.open("/nonexistent/file.tdms"));
    EXPECT_NE(f.error().find("cannot open"), std::string::npos);
}

TEST(FileReader, PropertiesOfEveryLevelAndTheChannels) {
    Builder meta;
    meta.U32(3);
    meta.ObjectNoRaw("/", 1).PropertyHeader("device", kTypeString).PString("rp");
    meta.ObjectNoRaw("/'G'", 1).PropertyHeader("decimation", kTypeU32).U32(8);
    meta.ObjectWithRaw("/'G'/'ch1'", kTypeI16, 3, 1).PropertyHeader("wf_increment", kTypeF64).F64(64e-9);
    Builder raw;
    raw.I16(-1).I16(0).I16(1);

    FileReader f;
    ASSERT_TRUE(Open(f, Seg(kFirst, meta, raw).bytes())) << f.error();
    EXPECT_EQ(TDMS::getProperty<std::string>(f.properties(), "device"), std::string("rp"));
    EXPECT_EQ(TDMS::getProperty<uint32_t>(f.groupProperties("G"), "decimation"), 8u);
    EXPECT_TRUE(f.groupProperties("missing").empty());
    EXPECT_EQ(f.groups(), std::vector<std::string>{"G"});
    ASSERT_EQ(f.channels().size(), 1u);
    const auto* c = f.channel("G", "ch1");
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->type, TDMSType::Integer16);
    EXPECT_EQ(c->count, 3u);
    EXPECT_EQ(TDMS::getProperty<double>(c->properties, "wf_increment"), 64e-9);
    EXPECT_EQ(f.readAll<int16_t>("G", "ch1"), (std::vector<int16_t>{-1, 0, 1}));
    EXPECT_EQ(f.channel("G", "ch2"), nullptr);
}

TEST(FileReader, ARawOnlySegmentRepeatsTheLayoutOfThePreviousOne) {
    Builder raw2;
    raw2.I16(3).I16(4).I32(101);
    FileReader f;
    ASSERT_TRUE(Open(f, Cat({FirstAB(), Seg(kTocRawData, Builder(), raw2)}))) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{1, 2, 3, 4}));
    EXPECT_EQ(f.readAll<int32_t>("G", "b"), (std::vector<int32_t>{100, 101}));
}

TEST(FileReader, AnObjectNotListedAgainKeepsItsSamples) {
    // segment 2 lists only b ("same index") without a new object list: a still has 2 samples there
    Builder meta;
    meta.U32(1).PString("/'G'/'b'").U32(kSameRawIndex).U32(0);
    Builder raw;
    raw.I16(5).I16(6).I32(102);
    FileReader f;
    ASSERT_TRUE(Open(f, Cat({FirstAB(), Seg(kTocMetaData | kTocRawData, meta, raw)}))) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{1, 2, 5, 6}));
    EXPECT_EQ(f.readAll<int32_t>("G", "b"), (std::vector<int32_t>{100, 102}));
}

TEST(FileReader, NoRawDataIndexMeansNoSamplesInThatSegment) {
    // segment 2 gives a only a new property: it has no samples there, b goes on
    Builder meta;
    meta.U32(1).ObjectNoRaw("/'G'/'a'", 1).PropertyHeader("note", kTypeString).PString("paused");
    Builder raw;
    raw.I32(103);
    FileReader f;
    ASSERT_TRUE(Open(f, Cat({FirstAB(), Seg(kTocMetaData | kTocRawData, meta, raw)}))) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{1, 2}));
    EXPECT_EQ(f.readAll<int32_t>("G", "b"), (std::vector<int32_t>{100, 103}));
    EXPECT_EQ(TDMS::getProperty<std::string>(f.channel("G", "a")->properties, "note"), std::string("paused"));
}

TEST(FileReader, ANewObjectListDropsTheObjectsItDoesNotName) {
    Builder meta;
    meta.U32(1).PString("/'G'/'b'").U32(kSameRawIndex).U32(0);
    Builder raw;
    raw.I32(104);
    FileReader f;
    ASSERT_TRUE(Open(f, Cat({FirstAB(), Seg(kFirst, meta, raw)}))) << f.error();
    EXPECT_EQ(f.channel("G", "a")->count, 2u);
    EXPECT_EQ(f.readAll<int32_t>("G", "b"), (std::vector<int32_t>{100, 104}));
}

TEST(FileReader, ASegmentHoldsSeveralChunksOfTheSameLayout) {
    Builder meta;
    meta.U32(2).ObjectWithRaw("/'G'/'a'", kTypeI16, 2, 0).ObjectWithRaw("/'G'/'b'", kTypeU8, 1, 0);
    Builder raw;
    for (int chunk = 0; chunk < 3; chunk++)
        raw.I16(int16_t(chunk * 10)).I16(int16_t(chunk * 10 + 1)).U8(uint8_t(chunk));
    FileReader f;
    ASSERT_TRUE(Open(f, Seg(kFirst, meta, raw).bytes())) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{0, 1, 10, 11, 20, 21}));
    EXPECT_EQ(f.readAll<uint8_t>("G", "b"), (std::vector<uint8_t>{0, 1, 2}));
}

TEST(FileReader, ACutOffChunkKeepsTheWholeSamplesThatAreThere) {
    Builder meta;
    meta.U32(2).ObjectWithRaw("/'G'/'a'", kTypeI16, 2, 0).ObjectWithRaw("/'G'/'b'", kTypeI16, 2, 0);
    Builder raw;
    raw.I16(1).I16(2).I16(3).I16(4);  // chunk 1
    raw.I16(5).I16(6).I16(7).U8(0);   // chunk 2: a whole, b one sample and a stray byte
    FileReader f;
    ASSERT_TRUE(Open(f, Seg(kFirst, meta, raw).bytes())) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{1, 2, 5, 6}));
    EXPECT_EQ(f.readAll<int16_t>("G", "b"), (std::vector<int16_t>{3, 4, 7}));
}

TEST(FileReader, AnIncompleteLastSegmentLastsToTheEndOfTheFile) {
    Builder meta;
    meta.U32(1).ObjectWithRaw("/'G'/'a'", kTypeI16, 1, 0);
    Builder b;
    b.LeadIn(kFirst, -1, static_cast<std::int64_t>(meta.size()));  // 0xFFFFFFFFFFFFFFFF: the writer never finished it
    b.Raw(meta.bytes()).I16(7).I16(8).I16(9);
    FileReader f;
    ASSERT_TRUE(Open(f, b.bytes())) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{7, 8, 9}));
}

TEST(FileReader, InterleavedChannelsAreReadByColumn) {
    Builder meta;
    meta.U32(2).ObjectWithRaw("/'G'/'a'", kTypeI16, 3, 0).ObjectWithRaw("/'G'/'b'", kTypeI32, 3, 0);
    Builder raw;
    for (int row = 0; row < 3; row++)
        raw.I16(int16_t(row)).I32(1000 + row);
    FileReader f;
    ASSERT_TRUE(Open(f, Seg(kFirst | kTocInterleaved, meta, raw).bytes())) << f.error();
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{0, 1, 2}));
    EXPECT_EQ(f.readAll<int32_t>("G", "b"), (std::vector<int32_t>{1000, 1001, 1002}));
    EXPECT_EQ(f.read<int32_t>("G", "b", 1, 5), (std::vector<int32_t>{1001, 1002}));
}

TEST(FileReader, ReadingARangeCrossesChunks) {
    Builder raw2;
    raw2.I16(3).I16(4).I32(101);
    FileReader f;
    ASSERT_TRUE(Open(f, Cat({FirstAB(), Seg(kTocRawData, Builder(), raw2)}))) << f.error();
    EXPECT_EQ(f.read<int16_t>("G", "a", 1, 2), (std::vector<int16_t>{2, 3}));
    EXPECT_EQ(f.read<int16_t>("G", "a", 3, 100), (std::vector<int16_t>{4}));
    EXPECT_TRUE(f.read<int16_t>("G", "a", 4, 1).empty());
    std::vector<std::uint8_t> bytes{0xEE};
    EXPECT_EQ(f.read("G", "a", 0, 2, bytes), 2u);
    EXPECT_EQ(bytes, (std::vector<std::uint8_t>{0xEE, 1, 0, 2, 0})) << "appended to what was there";
}

TEST(FileReader, TheWrongCppTypeReadsNothing) {
    FileReader f;
    ASSERT_TRUE(Open(f, FirstAB().bytes()));
    EXPECT_TRUE(f.readAll<int32_t>("G", "a").empty());
    EXPECT_TRUE(f.readAll<uint16_t>("G", "a").empty());
    EXPECT_TRUE(f.readAll<int16_t>("G", "missing").empty());
}

TEST(FileReader, ALaterPropertyReplacesTheEarlierOne) {
    Builder meta1;
    meta1.U32(1).ObjectNoRaw("/", 2).PropertyHeader("state", kTypeString).PString("start").PropertyHeader("n", kTypeI32).I32(1);
    Builder meta2;
    meta2.U32(1).ObjectNoRaw("/", 1).PropertyHeader("n", kTypeF64).F64(2.5);
    FileReader f;
    ASSERT_TRUE(Open(f, Cat({Seg(kTocMetaData | kTocNewObjList, meta1, Builder()), Seg(kTocMetaData, meta2, Builder())}))) << f.error();
    EXPECT_EQ(f.properties().at("state"), Value("start"));
    EXPECT_EQ(f.properties().at("n"), Value(2.5));
}

TEST(FileReader, NamesWithQuotesAreUnescaped) {
    Builder meta;
    meta.U32(1).ObjectWithRaw("/'it''s'/'a''b'", kTypeU8, 1, 0);
    Builder raw;
    raw.U8(5);
    FileReader f;
    ASSERT_TRUE(Open(f, Seg(kFirst, meta, raw).bytes())) << f.error();
    ASSERT_NE(f.channel("it's", "a'b"), nullptr);
    EXPECT_EQ(f.readAll<uint8_t>("it's", "a'b"), (std::vector<uint8_t>{5}));
}

TEST(FileReader, AStringChannelIsListedButNotRead) {
    Builder meta;
    meta.U32(2).ObjectWithStringRaw("/'G'/'s'", 2, 8 + 3, 0).ObjectWithRaw("/'G'/'a'", kTypeI16, 1, 0);
    Builder raw;
    raw.U32(1).U32(3).Raw(std::string("abc")).I16(42);
    FileReader f;
    ASSERT_TRUE(Open(f, Seg(kFirst, meta, raw).bytes())) << f.error();
    EXPECT_EQ(f.channel("G", "s")->type, TDMSType::String);
    EXPECT_EQ(f.channel("G", "s")->count, 2u);
    std::vector<std::uint8_t> out;
    EXPECT_EQ(f.read("G", "s", 0, 2, out), 0u);
    EXPECT_EQ(f.readAll<int16_t>("G", "a"), (std::vector<int16_t>{42}));
}

TEST(FileReader, TdmsVersion1AndAMetadataOnlySegmentWithoutRawOffset) {
    Builder meta;
    meta.U32(1).ObjectNoRaw("/", 1).PropertyHeader("k", kTypeI8).I8(3);
    Builder b;
    b.LeadIn(kTocMetaData | kTocNewObjList, static_cast<std::int64_t>(meta.size()), 0, 4712).Raw(meta.bytes());
    FileReader f;
    ASSERT_TRUE(Open(f, b.bytes())) << f.error();
    EXPECT_EQ(TDMS::getProperty<int>(f.properties(), "k"), 3);
}

TEST(FileReader, BytesAfterTheLastSegmentAreIgnored) {
    Bytes bytes = FirstAB().bytes();
    for (char c : std::string("padding that is not a segment at all"))
        bytes.push_back(static_cast<std::uint8_t>(c));
    FileReader f;
    ASSERT_TRUE(Open(f, bytes)) << f.error();
    EXPECT_EQ(f.channel("G", "a")->count, 2u);
}

TEST(FileReader, BigEndianAndDaqmxFilesAreRefused) {
    Builder meta;
    meta.U32(1).ObjectWithRaw("/'G'/'a'", kTypeI16, 1, 0);
    Builder raw;
    raw.I16(1);
    FileReader f;
    EXPECT_FALSE(Open(f, Seg(kFirst | kTocBigEndian, meta, raw).bytes()));
    EXPECT_NE(f.error().find("big-endian"), std::string::npos);
    EXPECT_FALSE(Open(f, Seg(kFirst | kTocDaqMx, meta, raw).bytes()));
    EXPECT_NE(f.error().find("DAQmx"), std::string::npos);

    Builder daqmx;
    daqmx.U32(1).PString("/'G'/'a'").U32(0x69120000).U32(0);
    EXPECT_FALSE(Open(f, Seg(kFirst, daqmx, raw).bytes()));
    EXPECT_NE(f.error().find("unsupported raw data index"), std::string::npos);
}

TEST(FileReader, BrokenMetadataIsAnErrorNotACrash) {
    FileReader f;
    Builder sameWithoutIndex;
    sameWithoutIndex.U32(1).PString("/'G'/'a'").U32(kSameRawIndex).U32(0);
    EXPECT_FALSE(Open(f, Seg(kFirst, sameWithoutIndex, Builder()).bytes()));
    EXPECT_NE(f.error().find("never had"), std::string::npos);

    Builder badPath;
    badPath.U32(1).ObjectNoRaw("G/a", 0);
    EXPECT_FALSE(Open(f, Seg(kTocMetaData, badPath, Builder()).bytes()));

    Builder groupWithData;
    groupWithData.U32(1).ObjectWithRaw("/'G'", kTypeI16, 1, 0);
    Builder raw;
    raw.I16(1);
    EXPECT_FALSE(Open(f, Seg(kFirst, groupWithData, raw).bytes()));

    Builder typeChange;
    typeChange.U32(1).ObjectWithRaw("/'G'/'a'", kTypeI32, 1, 0);
    Builder raw32;
    raw32.I32(1);
    EXPECT_FALSE(Open(f, Cat({FirstAB(), Seg(kTocMetaData | kTocRawData, typeChange, raw32)})));
    EXPECT_NE(f.error().find("changes its data type"), std::string::npos);
}

// Every prefix of a valid file and every byte of it set to a few values must open or fail
// cleanly, without a crash or a hang (run under ASan for memory errors).
TEST(FileReader, CorruptedFilesNeverCrashOrHang) {
    Builder raw2;
    raw2.I16(3).I16(4).I32(101);
    const Bytes good = Cat({FirstAB(), Seg(kTocRawData, Builder(), raw2)});
    const auto start = std::chrono::steady_clock::now();
    auto exercise = [](const Bytes& bytes) {
        FileReader f;
        if (Open(f, bytes)) {
            for (const auto& c : f.channels()) {
                std::vector<std::uint8_t> out;
                f.read(c.group, c.name, 0, c.count, out);
            }
        }
    };
    for (std::size_t n = 0; n <= good.size(); n++)
        exercise(Bytes(good.begin(), good.begin() + static_cast<std::ptrdiff_t>(n)));
    for (std::size_t i = 0; i < good.size(); i++) {
        for (std::uint8_t v : {0x00, 0x01, 0x7F, 0x80, 0xFF}) {
            Bytes bad = good;
            bad[i] = v;
            exercise(bad);
        }
    }
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(10));
}
