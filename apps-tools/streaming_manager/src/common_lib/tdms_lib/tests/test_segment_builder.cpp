/**
 * TDMS::SegmentBuilder against bytes stated by hand from the TDMS 2.0 specification
 * (spec_builder.h shares no code with the library), so the writer cannot agree with the reader
 * on a misunderstanding.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "byte_utils.h"
#include "spec_builder.h"
#include "tdms.h"

using namespace tdms_test;
using TDMS::SegmentBuilder;
using TDMS::TDMSType;
using TDMS::Value;

namespace {

auto Written(SegmentBuilder& s) -> Bytes {
    return ToBytes(s.bytes());
}

// A segment of the given metadata and raw bytes, as the builder writes it: metadata, new object
// list and, when there are samples, raw data
auto Segment(const Builder& meta, const Bytes& raw) -> Bytes {
    Builder b;
    const std::int32_t toc = kTocMetaData | kTocNewObjList | (raw.empty() ? 0 : kTocRawData);
    b.LeadIn(toc, static_cast<std::int64_t>(meta.size() + raw.size()), static_cast<std::int64_t>(meta.size()));
    b.Raw(meta.bytes()).Raw(raw);
    return b.bytes();
}

}  // namespace

TEST(SegmentBuilder, AnEmptyBuilderWritesNothing) {
    SegmentBuilder s;
    EXPECT_TRUE(s.empty());
    EXPECT_TRUE(s.bytes().empty());
}

TEST(SegmentBuilder, PropertiesAndSamplesOfOneChannel) {
    SegmentBuilder s;
    s.setProperty("name", "x");
    const std::vector<int16_t> samples{1, 2};
    s.copySamples("G", "c", samples);
    s.setChannelProperty("G", "c", "k", int32_t(7));

    Builder meta;
    meta.U32(3);
    meta.ObjectNoRaw("/", 1).PropertyHeader("name", kTypeString).PString("x");
    meta.ObjectNoRaw("/'G'", 0);
    meta.ObjectWithRaw("/'G'/'c'", kTypeI16, 2, 1).PropertyHeader("k", kTypeI32).I32(7);
    Builder raw;
    raw.I16(1).I16(2);
    ExpectBytesEq(Written(s), Segment(meta, raw.bytes()));
}

TEST(SegmentBuilder, APropertyOnlySegmentHasNoRawDataFlag) {
    SegmentBuilder s;
    s.setGroupProperty("G", "rate", 125e6);
    s.setChannelProperty("G", "c", "unit_string", "V");

    Builder meta;
    meta.U32(3);
    meta.ObjectNoRaw("/", 0);
    meta.ObjectNoRaw("/'G'", 1).PropertyHeader("rate", kTypeF64).F64(125e6);
    meta.ObjectNoRaw("/'G'/'c'", 1).PropertyHeader("unit_string", kTypeString).PString("V");
    ExpectBytesEq(Written(s), Segment(meta, {}));
}

TEST(SegmentBuilder, SamplesAddedTwiceFollowEachOther) {
    SegmentBuilder s;
    const int32_t a[2] = {1, 2};
    const int32_t b[1] = {3};
    s.copySamples("G", "c", a, 2);
    s.copySamples("G", "c", b, 1);

    Builder meta;
    meta.U32(3).ObjectNoRaw("/", 0).ObjectNoRaw("/'G'", 0).ObjectWithRaw("/'G'/'c'", kTypeI32, 3, 0);
    Builder raw;
    raw.I32(1).I32(2).I32(3);
    ExpectBytesEq(Written(s), Segment(meta, raw.bytes()));
}

TEST(SegmentBuilder, ObjectsAndSamplesKeepTheOrderOfFirstUse) {
    SegmentBuilder s;
    const uint8_t x[1] = {0xAA};
    const uint8_t y[1] = {0xBB};
    const uint8_t z[1] = {0xCC};
    s.copySamples("B", "y", y, 1);
    s.copySamples("A", "x", x, 1);
    s.copySamples("B", "z", z, 1);

    Builder meta;
    meta.U32(6).ObjectNoRaw("/", 0);
    meta.ObjectNoRaw("/'B'", 0).ObjectWithRaw("/'B'/'y'", kTypeU8, 1, 0).ObjectWithRaw("/'B'/'z'", kTypeU8, 1, 0);
    meta.ObjectNoRaw("/'A'", 0).ObjectWithRaw("/'A'/'x'", kTypeU8, 1, 0);
    ExpectBytesEq(Written(s), Segment(meta, Bytes{0xBB, 0xCC, 0xAA}));
}

TEST(SegmentBuilder, AQuoteInANameIsWrittenTwice) {
    SegmentBuilder s;
    s.setChannelProperty("it's", "a'b", "k", true);

    Builder meta;
    meta.U32(3).ObjectNoRaw("/", 0).ObjectNoRaw("/'it''s'", 0);
    meta.ObjectNoRaw("/'it''s'/'a''b'", 1).PropertyHeader("k", kTypeBool).U8(1);
    ExpectBytesEq(Written(s), Segment(meta, {}));
}

TEST(SegmentBuilder, EveryPropertyTypeHasItsPayload) {
    SegmentBuilder s;
    s.setProperty("i8", int8_t(-1));
    s.setProperty("u64", uint64_t(5));
    s.setProperty("f32", 0.5f);
    s.setProperty("ts", TDMS::TimeStamp{3, 1});

    // std::map orders the keys
    Builder meta;
    meta.U32(1).ObjectNoRaw("/", 4);
    meta.PropertyHeader("f32", kTypeF32).F32(0.5f);
    meta.PropertyHeader("i8", kTypeI8).I8(-1);
    meta.PropertyHeader("ts", kTypeTimeStamp).U64(1).I64(3);
    meta.PropertyHeader("u64", kTypeU64).U64(5);
    ExpectBytesEq(Written(s), Segment(meta, {}));
}

TEST(SegmentBuilder, ASharedBufferIsWrittenWithoutACopy) {
    SegmentBuilder s;
    std::shared_ptr<std::uint8_t[]> data(new std::uint8_t[4]{1, 2, 3, 4});
    s.addSamples("G", "c", TDMSType::Integer16, std::shared_ptr<const std::uint8_t[]>(data), 2);
    data[0] = 9;  // the builder writes what is in the buffer at write()
    EXPECT_EQ(Written(s).back(), 4);
    EXPECT_EQ(Written(s)[Written(s).size() - 4], 9);
}

TEST(SegmentBuilder, ASharedVectorIsWrittenWithoutACopy) {
    SegmentBuilder s;
    auto time = std::make_shared<std::vector<int64_t>>(std::vector<int64_t>{1, 2});
    s.addSamples("G", "time", time);
    (*time)[1] = 0x0102030405060708;  // what is in the vector at write()

    Builder meta;
    meta.U32(3).ObjectNoRaw("/", 0).ObjectNoRaw("/'G'", 0).ObjectWithRaw("/'G'/'time'", kTypeI64, 2, 0);
    Builder raw;
    raw.I64(1).I64(0x0102030405060708);
    ExpectBytesEq(Written(s), Segment(meta, raw.bytes()));
    EXPECT_EQ(time.use_count(), 2) << "the builder holds the vector itself";
    s.clear();
    EXPECT_EQ(time.use_count(), 1);
}

TEST(SegmentBuilder, PartsAreTheHeadAndTheSampleBlocksInPlace) {
    SegmentBuilder s;
    auto a = std::make_shared<std::vector<int16_t>>(std::vector<int16_t>{1, 2, 3});
    auto b = std::make_shared<const std::vector<float>>(std::vector<float>{0.5f});
    s.setProperty("k", int32_t(1));
    s.addSamples("G", "a", a);
    s.addSamples("G", "b", b);
    const auto parts = s.parts();
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[1].data, reinterpret_cast<const std::uint8_t*>(a->data())) << "no copy of the samples";
    EXPECT_EQ(parts[1].size, 6u);
    EXPECT_EQ(parts[2].data, reinterpret_cast<const std::uint8_t*>(b->data()));
    Bytes joined;
    for (const auto& p : parts)
        joined.insert(joined.end(), p.data, p.data + p.size);
    ExpectBytesEq(joined, Written(s));
    EXPECT_EQ(s.size(), joined.size());
    s.clear();
    EXPECT_TRUE(s.parts().empty());
    EXPECT_EQ(s.size(), 0u);
}

TEST(SegmentBuilder, ChannelDataNeedsAFixedWidthTypeAndOneTypePerSegment) {
    SegmentBuilder s;
    const int16_t v[1] = {1};
    EXPECT_THROW(s.copySamples("G", "c", TDMSType::String, v, 1), std::invalid_argument);
    s.copySamples("G", "c", v, 1);
    const int32_t w[1] = {1};
    EXPECT_THROW(s.copySamples("G", "c", w, 1), std::invalid_argument);
}

TEST(SegmentBuilder, ClearEmptiesIt) {
    SegmentBuilder s;
    s.setProperty("a", 1.0);
    s.clear();
    EXPECT_TRUE(s.empty());
    EXPECT_TRUE(s.bytes().empty());
}
