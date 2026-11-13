/**
 * BoardInfoMessage: the JSON carried by CS_RESPONSE_BOARD_INFO.
 */

#include <gtest/gtest.h>

#include "config_net_lib/board_info.h"

TEST(BoardInfo, EveryFieldSurvivesTheRoundTrip) {
    BoardInfoMessage source;
    source.model = 13;
    source.mac = "00:26:32:F0:F1:40";
    source.master = false;
    source.adcChannels = 4;
    source.dacChannels = 0;
    source.adcRate = 250000000;
    source.dacRate = 250000000;
    source.adcBits = 12;
    source.adcRanges = true;
    source.adcACDC = true;
    source.dacGain = true;
    source.adc = false;
    source.adcBlockMode = true;
    source.dac = false;
    source.dacTriggerOutputs = true;
    source.gpio = true;
    source.extTrigger = true;
    source.memorySize = 0x1F00000;
    source.memoryMaxBlock = 8388608;

    BoardInfoMessage target;
    ASSERT_TRUE(BoardInfoMessage::fromJson(source.toJson(), target));
    EXPECT_EQ(target, source);
}

TEST(BoardInfo, TheDefaultsSurviveTheRoundTrip) {
    BoardInfoMessage target;
    target.gpio = true;
    ASSERT_TRUE(BoardInfoMessage::fromJson(BoardInfoMessage().toJson(), target));
    EXPECT_EQ(target, BoardInfoMessage());
}

// An empty tag is what a server sends before it knows its board.
TEST(BoardInfo, NotABoardInfoIsRejectedAndLeavesTheTargetAlone) {
    BoardInfoMessage target;
    target.model = 7;
    EXPECT_FALSE(BoardInfoMessage::fromJson("", target));
    EXPECT_FALSE(BoardInfoMessage::fromJson("{", target));
    EXPECT_FALSE(BoardInfoMessage::fromJson("[]", target));
    EXPECT_FALSE(BoardInfoMessage::fromJson(R"({"adc_channels":5})", target)) << "four inputs at most";
    EXPECT_FALSE(BoardInfoMessage::fromJson(R"({"dac_channels":3})", target)) << "two outputs at most";
    EXPECT_FALSE(BoardInfoMessage::fromJson(R"({"model":256})", target));
    EXPECT_FALSE(BoardInfoMessage::fromJson(R"({"gpio":"yes"})", target));
    EXPECT_EQ(target.model, 7);
}

// Absent keys take the defaults and unknown keys are ignored, so either side may be newer.
TEST(BoardInfo, AbsentAndUnknownKeysAreAccepted) {
    BoardInfoMessage target;
    target.adcBits = 16;
    ASSERT_TRUE(BoardInfoMessage::fromJson(R"({"gpio":true,"from_the_future":1})", target));
    EXPECT_TRUE(target.gpio);
    EXPECT_EQ(target.adcBits, 14) << "the whole info is replaced, not merged";
}
