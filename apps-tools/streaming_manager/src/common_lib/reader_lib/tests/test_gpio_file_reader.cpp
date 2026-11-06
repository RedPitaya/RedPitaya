/**
 * The words of the GPIO output from a BIN stream file: a recording of the input or output words,
 * with and without RLE, repeated, with the gaps of the recording or without them.
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "bin_lib/bin_file.h"
#include "bin_lib/gpio_words.h"
#include "reader_lib/gpio_file_reader.h"

using namespace bin_lib;
using Repeat = CStreamSettings::GPIORepeat;

namespace {

auto Path(const std::string& name) -> std::string {
    auto dir = std::filesystem::temp_directory_path() / ("gpio_reader_" + std::to_string(::getpid()));
    std::filesystem::create_directories(dir);
    return (dir / name).string();
}

// A file of one channel of GPIO words, a block per buffer: {words, lost before}
auto Write(const std::string& path, SampleType type, bool rle, const std::vector<std::pair<std::vector<uint32_t>, uint64_t>>& buffers) -> void {
    FileInfo info;
    info.source = type == SampleType::GPIO_IN_WORD ? Source::GPIO_IN : Source::GPIO_OUT;
    info.channels = {{1, type, static_cast<uint16_t>(rle ? CHANNEL_FLAG_RLE : 0), 0}};
    auto out = fileHeader(info);
    Timeline line;
    for (const auto& [words, lost] : buffers) {
        BlockInfo b;
        b.samples = type == SampleType::GPIO_IN_WORD ? gpioInSamples(words.data(), words.size(), rle) : gpioOutSamples(words.data(), words.size(), rle);
        b.lost = lost;
        b.position = line.next(0, lost, 0, b.samples);
        appendBlock(out, b, words.data(), static_cast<uint32_t>(words.size() * 4));
    }
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
}

// Every word until the end, read in pieces of `piece`
auto Drain(CGpioFileReader& r, size_t piece = 3, int maxReads = 1000) -> std::vector<uint32_t> {
    std::vector<uint32_t> all;
    std::vector<uint32_t> buf(piece);
    for (int i = 0; i < maxReads; i++) {
        size_t got = 0;
        auto res = r.read(buf.data(), piece, got);
        all.insert(all.end(), buf.begin(), buf.begin() + static_cast<std::ptrdiff_t>(got));
        if (res == CGpioFileReader::Result::ENDED)
            break;
    }
    return all;
}

}  // namespace

TEST(GpioFileReader, AnInputRecordingPlaysItsLevels) {
    const auto path = Path("in.bin");
    // P 1 for 3, N 7 for 3; then after 2 lost P 2 for 2, N 8 for 2
    Write(path, SampleType::GPIO_IN_WORD, true, {{{inWord(1, 2, 7, 2)}, 0}, {{inWord(2, 1, 8, 1)}, 2}});
    CGpioFileReader r(path, Repeat::GPIO_REP_OFF, 0, true, false);
    ASSERT_EQ(r.isOpen(), CGpioFileReader::Open::OK) << r.error();
    EXPECT_EQ(Drain(r), (std::vector<uint32_t>{outWord(1, 7, 2), outWord(2, 8, 1)}));
}

TEST(GpioFileReader, WithItsGapsTheLinesHoldWhileTheRecordingMissedSamples) {
    const auto path = Path("gaps.bin");
    Write(path, SampleType::GPIO_IN_WORD, true, {{{inWord(1, 2, 7, 2)}, 0}, {{inWord(2, 1, 8, 1)}, 2}});
    CGpioFileReader r(path, Repeat::GPIO_REP_OFF, 0, true, true);
    EXPECT_EQ(Drain(r), (std::vector<uint32_t>{outWord(1, 7, 4), outWord(2, 8, 1)})) << "3 samples and 2 missing ones of the same lines";
}

TEST(GpioFileReader, WithoutRLEOnTheOutputAWordIsASample) {
    const auto path = Path("norle.bin");
    Write(path, SampleType::GPIO_IN_WORD, true, {{{inWord(1, 1, 7, 1)}, 0}});
    CGpioFileReader r(path, Repeat::GPIO_REP_OFF, 0, false, false);
    EXPECT_EQ(Drain(r), (std::vector<uint32_t>{outWord(1, 7, 0), outWord(1, 7, 0)}));
}

TEST(GpioFileReader, OutputWordsPlayAsTheyAreAndRepeat) {
    const auto path = Path("out.bin");
    Write(path, SampleType::GPIO_OUT_WORD, true, {{{outWord(1, 2, 9), outWord(3, 4, 0)}, 0}});
    CGpioFileReader r(path, Repeat::GPIO_REP_ON, 2, true, false);
    EXPECT_EQ(Drain(r), (std::vector<uint32_t>{outWord(1, 2, 9), outWord(3, 4, 0), outWord(1, 2, 9), outWord(3, 4, 0)}));
}

TEST(GpioFileReader, AnEndlessRepeatDoesNotEnd) {
    const auto path = Path("inf.bin");
    Write(path, SampleType::GPIO_OUT_WORD, true, {{{outWord(1, 2, 0), outWord(3, 4, 0)}, 0}});
    CGpioFileReader r(path, Repeat::GPIO_REP_INF, 0, true, false);
    std::vector<uint32_t> buf(100);
    size_t got = 0;
    EXPECT_EQ(r.read(buf.data(), 100, got), CGpioFileReader::Result::OK);
    EXPECT_EQ(got, 100u);
}

TEST(GpioFileReader, FilesTheOutputCannotPlay) {
    const auto adc = Path("adc.bin");
    FileInfo info;
    info.channels = {{1, SampleType::INT16, 0, 14}};
    auto bytes = fileHeader(info);
    std::ofstream(adc, std::ios::binary).write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    EXPECT_EQ(CGpioFileReader(adc, Repeat::GPIO_REP_OFF, 0, true, false).isOpen(), CGpioFileReader::Open::WRONG_TYPE);

    const auto empty = Path("empty.bin");
    Write(empty, SampleType::GPIO_OUT_WORD, true, {});
    EXPECT_EQ(CGpioFileReader(empty, Repeat::GPIO_REP_OFF, 0, true, false).isOpen(), CGpioFileReader::Open::EMPTY);

    EXPECT_EQ(CGpioFileReader(Path("none.bin"), Repeat::GPIO_REP_OFF, 0, true, false).isOpen(), CGpioFileReader::Open::CLOSE);
}
