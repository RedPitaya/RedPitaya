/**
 * The GPIO words: the two lanes of the input on one time line, the runs back to output words.
 */

#include <gtest/gtest.h>

#include <vector>

#include "bin_lib/gpio_words.h"

using namespace bin_lib;

namespace {

auto Decode(GpioInDecoder& d, const std::vector<uint32_t>& words) -> std::vector<GpioRun> {
    std::vector<GpioRun> runs;
    d.feed(words.data(), words.size(), [&](const GpioRun& r) { runs.push_back(r); });
    return runs;
}

auto Same(const std::vector<GpioRun>& a, const std::vector<GpioRun>& b) -> bool {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); i++)
        if (a[i].samples != b[i].samples || a[i].p != b[i].p || a[i].n != b[i].n)
            return false;
    return true;
}

}  // namespace

TEST(GpioWords, TheFieldsOfTheWords) {
    const uint32_t in = inWord(0xA5, 3, 0x5A, 7);
    EXPECT_EQ(inP(in), 0xA5);
    EXPECT_EQ(inPCount(in), 3u);
    EXPECT_EQ(inN(in), 0x5A);
    EXPECT_EQ(inNCount(in), 7u);
    const uint32_t out = outWord(0x12, 0x34, 255);
    EXPECT_EQ(out, 0x123400FFu);
    EXPECT_EQ(outP(out), 0x12);
    EXPECT_EQ(outN(out), 0x34);
    EXPECT_EQ(outCount(out), 255u);
}

TEST(GpioWords, TheLanesRunOnTheirOwnAndMeetOnOneTimeLine) {
    GpioInDecoder d(true);
    // P: 1 for 4, 2 for 2;  N: 7 for 2, 8 for 4
    auto runs = Decode(d, {inWord(1, 3, 7, 1), inWord(2, 1, 8, 3)});
    EXPECT_TRUE(Same(runs, {{2, 1, 7}, {2, 1, 8}, {2, 2, 8}}));
    EXPECT_EQ(d.samples(), 6u);
}

TEST(GpioWords, ARunGoesOnInTheNextBuffer) {
    GpioInDecoder d(true);
    // P is 1 for 2 then 3 for 4; N is 5 for 6: the second P run comes with the next buffer
    auto first = Decode(d, {inWord(1, 1, 5, 5)});
    EXPECT_TRUE(Same(first, {{2, 1, 5}}));
    auto second = Decode(d, {inWord(3, 3, 9, 0)});
    EXPECT_TRUE(Same(second, {{4, 3, 5}}));
    auto third = Decode(d, {inWord(4, 0, 9, 0)});
    EXPECT_TRUE(Same(third, {{1, 4, 9}}));
}

TEST(GpioWords, WithoutRLEAWordIsOneSample) {
    GpioInDecoder d(false);
    auto runs = Decode(d, {inWord(1, 9, 2, 9), inWord(3, 9, 4, 9)});
    EXPECT_TRUE(Same(runs, {{1, 1, 2}, {1, 3, 4}}));
    EXPECT_EQ(d.samples(), 2u);
}

TEST(GpioWords, TheRunsBecomeOutputWordsOf256SamplesAtMost) {
    GpioOutEncoder e(true);
    std::vector<uint32_t> out;
    e.add({300, 1, 2}, out);
    e.add({100, 1, 2}, out);  // the same levels: one run
    e.add({1, 3, 4}, out);
    e.flush(out);
    EXPECT_EQ(out, (std::vector<uint32_t>{outWord(1, 2, 255), outWord(1, 2, 143), outWord(3, 4, 0)}));
    EXPECT_EQ(gpioOutSamples(out.data(), out.size(), true), 401u);
}

TEST(GpioWords, WithoutRLEAnOutputWordIsOneSample) {
    GpioOutEncoder e(false);
    std::vector<uint32_t> out;
    e.add({3, 1, 2}, out);
    e.flush(out);
    EXPECT_EQ(out, std::vector<uint32_t>(3, outWord(1, 2, 0)));
}

TEST(GpioWords, AnInputRecordingPlaysAsTheSameLevels) {
    // input -> runs -> output -> the samples of the levels again
    GpioInDecoder d(true);
    GpioOutEncoder e(true);
    std::vector<uint32_t> out;
    d.feed(std::vector<uint32_t>{inWord(0x11, 9, 0x22, 4), inWord(0x33, 0, 0x44, 5)}.data(), 2, [&](const GpioRun& r) { e.add(r, out); });
    e.flush(out);
    std::vector<std::pair<uint8_t, uint8_t>> levels;
    for (uint32_t w : out)
        for (uint32_t k = 0; k <= outCount(w); k++)
            levels.push_back({outP(w), outN(w)});
    std::vector<std::pair<uint8_t, uint8_t>> expect;
    for (int i = 0; i < 5; i++)
        expect.push_back({0x11, 0x22});
    for (int i = 0; i < 5; i++)
        expect.push_back({0x11, 0x44});
    expect.push_back({0x33, 0x44});
    EXPECT_EQ(levels, expect);
}
