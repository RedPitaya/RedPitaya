/**
 * CHardware, the facade over the stream_app blocks: what it gives the scope for a configuration
 * (calibration of the board for the front end, its place before or after the filter, the
 * decimation), the trigger settings, the order rules (nothing while running), and the loss of a
 * buffer apart from the trigger wait. The dummy COscilloscope of the host build records what it
 * is given; the dummy board of board_lib gives the calibration and records the front end.
 */

#include <gtest/gtest.h>

#include "board_lib/board.h"
#include "uio_lib/hardware.h"
#include "uio_lib/oscilloscope_dummy.h"
#include "uio_lib/trigger.h"

using namespace uio_lib;
using board_lib::CDummyBoard;

namespace {

class Hardware : public ::testing::Test {
   protected:
    void SetUp() override { dummy::resetOscState(); }
    void TearDown() override { dummy::resetOscState(); }

    auto make(BoardInfo info = {}) -> CHardware::Ptr {
        board = std::make_shared<CDummyBoard>(info);
        return CHardware::create(board);
    }

    std::shared_ptr<CDummyBoard> board;
};

}  // namespace

TEST_F(Hardware, TheCreationStopsTheScopeOnItsOwnEvents) {
    auto hw = make();
    ASSERT_TRUE(hw);
    EXPECT_EQ(dummy::oscState().stops, 1) << "after the bitstream load every block listens to DAC A";
    EXPECT_FALSE(dummy::oscState().running);
    EXPECT_TRUE(hw->features().adc);
}

TEST_F(Hardware, TheBoardCalibrationOfTheFrontEndGoesToTheScope) {
    BoardInfo info;
    info.adcFilter = true;
    auto hw = make(info);
    board->adc[0][false][true] = {12, 1.25, 1, 2, 3, 4};   // CH1 1:1 DC
    board->adc[1][true][false] = {-30, 0.75, 5, 6, 7, 8};  // CH2 1:20 AC
    board->place = ADCCalibrationPlace::BEFORE_FILTER;

    ADCConfig cfg;
    cfg.decimation = 32;
    cfg.channels = {true, true, false, false};
    cfg.att1_20 = {false, true, false, false};
    cfg.dc = {true, false, true, true};
    cfg.eightBit = true;
    cfg.bufferBytes = 4096;
    ASSERT_TRUE(hw->adcConfigure(cfg));

    auto& s = dummy::oscState();
    EXPECT_EQ(s.decimation, 32u);
    EXPECT_TRUE(s.eightBit);
    EXPECT_FALSE(s.filterBypass) << "the board has the filter";
    EXPECT_EQ(s.dataSize, 4096u);
    EXPECT_EQ(board->calibrationLoads, 1) << "read again for every run";
    EXPECT_EQ(s.calibration[0].offset, 12);
    EXPECT_FLOAT_EQ(s.calibration[0].gain, 1.25f);
    EXPECT_TRUE(s.calibration[0].beforeFilter);
    EXPECT_EQ(s.calibration[0].kk, 3);
    EXPECT_EQ(s.calibration[1].offset, -30) << "the 1:20 AC calibration of CH2";
    EXPECT_EQ(s.calibration[1].pp, 8);
    EXPECT_TRUE(board->frontEnd[1].set);
    EXPECT_TRUE(board->frontEnd[1].att1_20);
    EXPECT_FALSE(board->frontEnd[1].dc);
}

TEST_F(Hardware, CalibrationUpToV5IsAppliedAfterTheFilter) {
    auto hw = make();
    board->place = ADCCalibrationPlace::AFTER_FILTER;
    board->adc[0][false][true] = {5, 1.1};
    ASSERT_TRUE(hw->adcConfigure(ADCConfig()));
    EXPECT_FALSE(dummy::oscState().calibration[0].beforeFilter);
    EXPECT_EQ(dummy::oscState().calibration[0].offset, 5);
}

TEST_F(Hardware, WithoutTheFilterOrTheCalibrationTheCodesPassUnchanged) {
    auto hw = make();  // no filter in this FPGA
    board->adc[0][false][true] = {7, 1.5, 1, 2, 3, 4};
    ASSERT_TRUE(hw->adcConfigure(ADCConfig()));
    auto& s = dummy::oscState();
    EXPECT_TRUE(s.filterBypass);
    EXPECT_EQ(s.calibration[0].kk, 0xFFFFFF) << "a pass-through filter";
    EXPECT_EQ(s.calibration[0].aa, 0);

    ADCConfig raw;
    raw.calibration = false;
    ASSERT_TRUE(hw->adcConfigure(raw));
    EXPECT_EQ(s.calibration[0].offset, 0);
    EXPECT_FLOAT_EQ(s.calibration[0].gain, 1.0f);
    EXPECT_EQ(board->calibrationLoads, 1) << "not read without the calibration";
}

TEST_F(Hardware, AnUnreadableCalibrationRefusesTheConfiguration) {
    auto hw = make();
    board->calibrationReadable = false;
    EXPECT_FALSE(hw->adcConfigure(ADCConfig()));
}

TEST_F(Hardware, NothingIsSetWhileTheADCRuns) {
    auto hw = make();
    ASSERT_TRUE(hw->adcConfigure(ADCConfig()));
    hw->adcStart();
    EXPECT_TRUE(dummy::oscState().running);
    EXPECT_FALSE(hw->adcConfigure(ADCConfig()));
    EXPECT_FALSE(hw->adcSetTrigger(ADCTrigger()));
    hw->adcStop();
    EXPECT_TRUE(hw->adcSetTrigger(ADCTrigger()));
}

TEST_F(Hardware, TheTriggerSettingsReachTheScope) {
    dummy::oscState().blockMode = true;
    auto hw = make();
    ADCTrigger trig;
    trig.enable = true;
    trig.sources = TRIG_SRC_ADC_CH2 | TRIG_SRC_EXT;
    trig.low = -100;
    trig.high = 200;
    trig.falling = true;
    trig.blocks = 4;
    ASSERT_TRUE(hw->adcSetTrigger(trig));
    auto& s = dummy::oscState();
    EXPECT_TRUE(s.triggerMode);
    EXPECT_EQ(s.triggerSources, TRIG_SRC_ADC_CH2 | TRIG_SRC_EXT);
    EXPECT_EQ(s.triggerLow, -100);
    EXPECT_EQ(s.triggerHigh, 200);
    EXPECT_TRUE(s.triggerFalling);
    EXPECT_EQ(s.triggerBlocks, 4);
    EXPECT_EQ(s.eventSource, EVENT_SRC_OSC1) << "OWN is the scope's own events";
    EXPECT_EQ(hw->adcTrigger().blocks, 4);

    trig.events = EventSource::GEN_A;
    ASSERT_TRUE(hw->adcSetTrigger(trig));
    EXPECT_EQ(s.eventSource, EVENT_SRC_GEN_A);
}

TEST_F(Hardware, BadTriggerSettingsAreRefused) {
    auto hw = make();  // no block mode in this FPGA
    ADCTrigger blocks;
    blocks.enable = true;
    blocks.blocks = 2;
    EXPECT_FALSE(hw->adcSetTrigger(blocks));
    ADCTrigger levels;
    levels.enable = true;
    levels.low = 10;
    levels.high = -10;
    EXPECT_FALSE(hw->adcSetTrigger(levels)) << "low above high";
    EXPECT_EQ(hw->adcTrigger().blocks, 0) << "a refused setting is not kept";
}

TEST_F(Hardware, TheLossOfABufferComesWithoutTheTriggerWait) {
    auto& s = dummy::oscState();
    s.lost = [](uint8_t index, uint8_t ch) -> uint64_t { return index == 1 ? 5000 + ch : 0; };
    s.waitCount = [](uint8_t index) -> uint64_t { return index == 1 ? 4000 : 0; };
    s.time = [](uint8_t index) -> int64_t { return 1000 + index; };
    s.triggerCount = []() -> uint32_t { return 7; };
    auto hw = make();
    auto b = hw->adcBuffer(1);
    EXPECT_EQ(b.lost[0], 1000u);
    EXPECT_EQ(b.lost[3], 1003u);
    EXPECT_EQ(b.wait, 4000u);
    EXPECT_EQ(b.time, 1001);
    EXPECT_EQ(b.triggerCount, 7u);
}

TEST_F(Hardware, TheSoftwareTriggerAndTheStatus) {
    dummy::oscState().armed = true;
    dummy::oscState().triggerCount = []() -> uint32_t { return 3; };
    auto hw = make();
    hw->adcSoftwareTrigger();
    EXPECT_EQ(dummy::oscState().softwareTriggers, 1);
    auto st = hw->adcStatus();
    EXPECT_TRUE(st.armed);
    EXPECT_EQ(st.triggerCount, 3u);
}

TEST_F(Hardware, TheDACGainIsSwitchedOnlyWhereTheBoardHasIt) {
    auto hw = make();
    DACConfig cfg;
    cfg.gain5x = {true, false};
    ASSERT_TRUE(hw->dacConfigure(cfg));
    EXPECT_EQ(board->dacGain[0], -1) << "no gain switch";

    BoardInfo info;
    info.dacGain = true;
    hw = make(info);
    ASSERT_TRUE(hw->dacConfigure(cfg));
    EXPECT_EQ(board->dacGain[0], 1);
    EXPECT_EQ(board->dacGain[1], 0);
}

TEST_F(Hardware, OutsideTheTriggerModeTheSourcesAreTheDefaultOnes) {
    dummy::oscState().blockMode = true;
    auto hw = make();
    ADCTrigger on;
    on.enable = true;
    on.sources = TRIG_SRC_LA;
    on.blocks = 3;
    ASSERT_TRUE(hw->adcSetTrigger(on));
    EXPECT_FALSE(dummy::oscState().triggerSourcesDefault);

    ADCTrigger off = on;  // the settings kept, the mode off
    off.enable = false;
    off.low = 10;
    off.high = -10;  // not a trigger level outside the trigger mode
    off.events = EventSource::GEN_B;
    ASSERT_TRUE(hw->adcSetTrigger(off));
    auto& s = dummy::oscState();
    EXPECT_TRUE(s.triggerSourcesDefault) << "a daisy chain slave starts on the external trigger";
    EXPECT_EQ(s.triggerBlocks, 0);
    EXPECT_EQ(hw->adcTrigger().blocks, 0) << "no blocks in the buffer headers";
    EXPECT_EQ(s.eventSource, EVENT_SRC_GEN_B) << "a start following another block works outside the trigger mode";
}
