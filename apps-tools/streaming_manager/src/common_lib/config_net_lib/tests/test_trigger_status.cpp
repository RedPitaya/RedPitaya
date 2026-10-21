/**
 * TriggerStatus: the JSON carried by CS_RESPONSE_TRIGGER_STATUS.
 */

#include <gtest/gtest.h>

#include "config_net_lib/trigger_status.h"

TEST(TriggerStatus, EveryFieldSurvivesTheRoundTrip) {
    TriggerStatus source;
    source.adcArmed = true;
    source.adcTriggerCount = 0xFFFFFFFFu;
    source.dacArmed = 2;
    source.gpioInArmed = true;
    source.gpioOutArmed = true;
    source.gpioInTriggerCount = 12345;

    TriggerStatus target;
    ASSERT_TRUE(TriggerStatus::fromJson(source.toJson(), target));
    EXPECT_EQ(target, source);
}

TEST(TriggerStatus, TheDefaultsSurviveTheRoundTrip) {
    TriggerStatus target;
    target.adcArmed = true;
    ASSERT_TRUE(TriggerStatus::fromJson(TriggerStatus().toJson(), target));
    EXPECT_EQ(target, TriggerStatus());
}

// An empty tag is what a server without a status handler sends.
TEST(TriggerStatus, NotAStatusIsRejectedAndLeavesTheTargetAlone) {
    TriggerStatus target;
    target.adcTriggerCount = 7;
    EXPECT_FALSE(TriggerStatus::fromJson("", target));
    EXPECT_FALSE(TriggerStatus::fromJson("{", target));
    EXPECT_FALSE(TriggerStatus::fromJson("[]", target));
    EXPECT_FALSE(TriggerStatus::fromJson(R"({"adc_trigger_count":-1})", target));
    EXPECT_FALSE(TriggerStatus::fromJson(R"({"dac_armed":4})", target)) << "two DAC channels, bits 1:0";
    EXPECT_FALSE(TriggerStatus::fromJson(R"({"adc_armed":"yes"})", target));
    EXPECT_EQ(target.adcTriggerCount, 7u);
}

// Absent keys take the defaults and unknown keys are ignored, so either side may be newer.
TEST(TriggerStatus, AbsentAndUnknownKeysAreAccepted) {
    TriggerStatus target;
    target.gpioInTriggerCount = 9;
    ASSERT_TRUE(TriggerStatus::fromJson(R"({"adc_armed":true,"from_the_future":1})", target));
    EXPECT_TRUE(target.adcArmed);
    EXPECT_EQ(target.gpioInTriggerCount, 0u) << "the whole status is replaced, not merged";
}
