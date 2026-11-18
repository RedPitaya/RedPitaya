/**
 * RecordStatus: the JSON carried by CS_RESPONSE_RECORD_STATUS (the recordings of a board on its
 * SD card, item 1.5 of the streaming improvements).
 */

#include <gtest/gtest.h>

#include "config_net_lib/record_status.h"

TEST(RecordStatus, EveryFieldSurvivesTheRoundTrip) {
    RecordStatus source;
    source.adc.mode = StreamRecordStatus::SD;
    source.adc.running = true;
    source.adc.samples = 0x123456789ull;  // above 32 bits
    source.adc.limit = 250000000;
    source.adc.written = 0x2'0000'0000ull;
    source.adc.writeRate = 12.5e6;
    source.adc.queue = 0.25;
    source.adc.lostBoard = 1245184;
    source.adc.lostQueue = 262144;
    source.adc.file = "/home/redpitaya/streaming_files/adc/data_file.bin";
    source.gpioIn.mode = StreamRecordStatus::NETWORK;
    source.gpioIn.samples = 7;
    source.gpioIn.lostBoard = 3;
    source.sdFree = 26950000000ull;
    source.sdTotal = 30310000000ull;

    RecordStatus target;
    ASSERT_TRUE(RecordStatus::fromJson(source.toJson(), target));
    EXPECT_EQ(target, source);
}

// An empty tag is what a server without a handler sends.
TEST(RecordStatus, NotAStatusIsRejectedAndLeavesTheTargetAlone) {
    RecordStatus target;
    target.sdFree = 9;
    EXPECT_FALSE(RecordStatus::fromJson("", target));
    EXPECT_FALSE(RecordStatus::fromJson("{", target));
    EXPECT_FALSE(RecordStatus::fromJson("[]", target));
    EXPECT_FALSE(RecordStatus::fromJson(R"({"adc":{"mode":3}})", target)) << "none, network, SD";
    EXPECT_FALSE(RecordStatus::fromJson(R"({"adc":5})", target));
    EXPECT_FALSE(RecordStatus::fromJson(R"({"sd_free":-1})", target));
    EXPECT_EQ(target.sdFree, 9u);
}

// Absent keys take the defaults and unknown keys are ignored, so either side may be newer.
TEST(RecordStatus, AbsentAndUnknownKeysAreAccepted) {
    RecordStatus target;
    target.gpioIn.samples = 9;
    ASSERT_TRUE(RecordStatus::fromJson(R"({"adc":{"mode":2,"running":true,"from_the_future":1},"next":{}})", target));
    EXPECT_EQ(target.adc.mode, StreamRecordStatus::SD);
    EXPECT_TRUE(target.adc.running);
    EXPECT_EQ(target.gpioIn.samples, 0u) << "the whole status is replaced, not merged";
}
