/**
 * TDMS files of several segment shapes, drained through CReaderController.
 *
 * These began as regression tests for the metadata cursor of the former reader,
 * which skipped record 0 of every segment after the first: invisible for the
 * files this project writes (object 0 is the group) but a lost chunk for any
 * file whose segments begin with a channel, the layout of NI's incremental
 * metadata. The reader now goes through TDMS::FileReader; the same files still
 * have to come out whole and in order.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "support.h"

using namespace reader_test;

namespace {

constexpr int kSamplesPerSegment = 4;
constexpr std::uint32_t kBlockSize = kSamplesPerSegment * sizeof(std::int16_t);

auto MakeController(const std::string& path) -> CReaderController::Ptr {
    const CStreamSettings::DataFormat format = CStreamSettings::DataFormat::TDMS;
    const CStreamSettings::DACRepeat repeat = CStreamSettings::DACRepeat::DAC_REP_OFF;
    return CReaderController::Create(format, path, repeat, 1, kBlockSize);
}

}  // namespace

// A failure in any of the drain tests below could sit either in tdms_lib (the
// file was not parsed) or in CReaderController (it was parsed and then
// mishandled). This test pins the tdms_lib side on its own, so the two are never
// confused: it asserts the channel-first layout is fully readable through
// TDMS::File before any controller is involved.
TEST(MetadataCursor, AChannelFirstFileIsFullyReadableAtTheTdmsLayer) {
    TempFile file("channel_first_layer.tdms");
    FileShape shape;
    shape.withGroup = false;
    shape.segments = 3;
    shape.samplesPerSegment = kSamplesPerSegment;
    WriteFile(file.path(), shape);

    const std::string dump = DescribeTdmsFile(file.path());
    SCOPED_TRACE(dump);

    TDMS::File tdms;
    const auto segments = tdms.ReadFileWithoutClose(file.path());
    ASSERT_EQ(segments.size(), static_cast<std::size_t>(shape.segments));

    for (std::size_t s = 0; s < segments.size(); s++) {
        const auto records = tdms.GetMetadata(segments[s]);
        int carrying = 0;
        for (const auto& record : records) {
            if (record->PathStr != "/'Group'/'ch1'") {
                continue;
            }
            carrying++;
            EXPECT_EQ(record->RawData.Size, static_cast<std::int64_t>(kBlockSize)) << "segment " << s;
            EXPECT_EQ(record->RawData.DataType.GetDataType(), TDMS::TDMSType::Integer16) << "segment " << s;
            std::int64_t rawBytes = 0;
            for (const auto& block : record->RawData.DataType.GetRawVector()) {
                rawBytes += static_cast<std::int64_t>(block ? block->size : 0);
            }
            // RawData.Size is what checkTDMSFile and moveNextMetadata both test,
            // but getBufferTdms copies out of GetRawVector(). A nonzero Size with
            // an empty raw vector passes every guard and then yields a zero-byte
            // block, so the two have to be asserted separately.
            EXPECT_EQ(rawBytes, static_cast<std::int64_t>(kBlockSize)) << "segment " << s;
        }
        EXPECT_EQ(carrying, 1) << "segment " << s << " must expose exactly one ch1 record";
    }
    tdms.Close();
}

TEST(MetadataCursor, AllSegmentsAreReadWhenEverySegmentStartsWithAChannel) {
    // The regression: three segments, each whose FIRST object is the channel.
    // Before the fix only the first segment's samples came back.
    TempFile file("channel_first.tdms");
    FileShape shape;
    shape.withGroup = false;
    shape.segments = 3;
    shape.samplesPerSegment = kSamplesPerSegment;
    const auto expected = WriteFile(file.path(), shape);

    auto controller = MakeController(file.path());
    ASSERT_EQ(controller->isOpen(), CReaderController::OR_OK);
    const auto trace = DrainChannel1Traced(*controller, kBlockSize);
    EXPECT_EQ(trace.samples, expected) << trace.report << DescribeTdmsFile(file.path());
}

TEST(MetadataCursor, AllSegmentsAreReadWhenEverySegmentStartsWithAGroup) {
    // The shape this project's own writer produces. Behaviour must be unchanged.
    TempFile file("group_first.tdms");
    FileShape shape;
    shape.withGroup = true;
    shape.segments = 3;
    shape.samplesPerSegment = kSamplesPerSegment;
    const auto expected = WriteFile(file.path(), shape);

    auto controller = MakeController(file.path());
    ASSERT_EQ(controller->isOpen(), CReaderController::OR_OK);
    const auto trace = DrainChannel1Traced(*controller, kBlockSize);
    EXPECT_EQ(trace.samples, expected) << trace.report << DescribeTdmsFile(file.path());
}

TEST(MetadataCursor, ASingleSegmentIsReadInBothShapes) {
    for (bool withGroup : {false, true}) {
        SCOPED_TRACE(withGroup ? "group + channel" : "channel only");
        TempFile file("single.tdms");
        FileShape shape;
        shape.withGroup = withGroup;
        shape.segments = 1;
        shape.samplesPerSegment = kSamplesPerSegment;
        const auto expected = WriteFile(file.path(), shape);

        auto controller = MakeController(file.path());
        ASSERT_EQ(controller->isOpen(), CReaderController::OR_OK);
        const auto trace = DrainChannel1Traced(*controller, kBlockSize);
        EXPECT_EQ(trace.samples, expected) << trace.report << DescribeTdmsFile(file.path());
    }
}

TEST(MetadataCursor, ManySegmentsAreAllRead) {
    // Enough segments that an off-by-one in the cursor cannot be masked by luck.
    TempFile file("many.tdms");
    FileShape shape;
    shape.withGroup = false;
    shape.segments = 10;
    shape.samplesPerSegment = kSamplesPerSegment;
    const auto expected = WriteFile(file.path(), shape);

    auto controller = MakeController(file.path());
    ASSERT_EQ(controller->isOpen(), CReaderController::OR_OK);
    const auto trace = DrainChannel1Traced(*controller, kBlockSize);
    EXPECT_EQ(trace.samples.size(), expected.size()) << trace.report << DescribeTdmsFile(file.path());
    EXPECT_EQ(trace.samples, expected) << trace.report;
}

TEST(MetadataCursor, AMissingFileIsReportedRatherThanCrashing) {
    auto controller = MakeController("/nonexistent/definitely/not/here.tdms");
    EXPECT_NE(controller->isOpen(), CReaderController::OR_OK);

    CReaderController::Data data;
    std::vector<std::uint8_t> block(kBlockSize);
    EXPECT_EQ(controller->readBlock({block.data(), nullptr}, data), CReaderController::BR_EMPTY);
}

TEST(MetadataCursor, AnEmptyFileIsReportedRatherThanCrashing) {
    TempFile file("empty.tdms");
    { std::ofstream create(file.path(), std::ios::binary | std::ios::trunc); }

    auto controller = MakeController(file.path());
    EXPECT_NE(controller->isOpen(), CReaderController::OR_OK);

    CReaderController::Data data;
    std::vector<std::uint8_t> block(kBlockSize);
    EXPECT_EQ(controller->readBlock({block.data(), nullptr}, data), CReaderController::BR_EMPTY);
}

TEST(MetadataCursor, RepeatModeRereadsFromTheFirstSegment) {
    // resetReadFromBuffer used to leave the cached metadata vector of the LAST
    // segment in place, so a rewind resumed from the wrong segment.
    TempFile file("repeat.tdms");
    FileShape shape;
    shape.withGroup = false;
    shape.segments = 2;
    shape.samplesPerSegment = kSamplesPerSegment;
    const auto expected = WriteFile(file.path(), shape);

    const CStreamSettings::DataFormat format = CStreamSettings::DataFormat::TDMS;
    const CStreamSettings::DACRepeat repeat = CStreamSettings::DACRepeat::DAC_REP_ON;
    auto controller = CReaderController::Create(format, file.path(), repeat, 2, kBlockSize);
    ASSERT_EQ(controller->isOpen(), CReaderController::OR_OK);

    const auto trace = DrainChannel1Traced(*controller, kBlockSize);
    ASSERT_GE(trace.samples.size(), expected.size()) << "at least one full pass" << trace.report << DescribeTdmsFile(file.path());
    // The first pass must be the file in order, from its very first segment.
    const std::vector<std::int16_t> firstPass(trace.samples.begin(), trace.samples.begin() + static_cast<std::ptrdiff_t>(expected.size()));
    EXPECT_EQ(firstPass, expected) << trace.report;
}
