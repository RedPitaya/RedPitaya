/**
 * The BIN stream file to CSV, as rpsa_client records CSV: a row per sample, an index column, the
 * lost samples as "-", the time of a sample from the start of the recording and its position.
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "bin_lib/bin_file.h"
#include "converter_lib/converter.h"

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
    EXPECT_EQ(ReadText(Path("rec.csv")), "1,1,-1\n2,2,-2\n3,11,-11\n4,12,-12\n5,-,-\n");
}

TEST(ConvertToCSV, TheTimeComesFromTheStartAndThePosition) {
    const auto bin = Path("time.bin");
    WriteRecording(bin, 1000000000);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToCSV(bin, "", static_cast<FH_CSVMode>(FH_CSV_ADD_TIME_COL_NS)));
    EXPECT_EQ(ReadText(Path("time.csv")), "1000000000,1,-1\n1000001000,2,-2\n1000003000,11,-11\n1000004000,12,-12\n1000005000,-,-\n")
        << "the second pack starts at position 3, after the lost sample";
}

TEST(ConvertToCSV, TheSegmentsArePacks) {
    const auto bin = Path("seg.bin");
    WriteRecording(bin);
    converter_lib::CConverter c;
    ASSERT_TRUE(c.convertToCSV(bin, 2, 2, "", FH_CSV_ADD_INDEX));
    EXPECT_EQ(ReadText(Path("seg.csv")), "3,11,-11\n4,12,-12\n5,-,-\n") << "the index counts the skipped pack";
}

TEST(ConvertToCSV, NotABinStreamFile) {
    const auto bin = Path("bad.bin");
    std::ofstream(bin) << "old segments";
    converter_lib::CConverter c;
    EXPECT_FALSE(c.convertToCSV(bin, "", FH_CSV_ADD_INDEX));
}
