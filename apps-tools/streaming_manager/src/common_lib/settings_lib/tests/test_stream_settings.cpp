/**
 * CStreamSettings: defaults, typed accessors and value semantics.
 *
 * Every ADC/DAC channel accessor here is ONE-BASED and range-checked, which is
 * the opposite of the zero-based convention data_lib uses for the same
 * channels, so the boundary cases (0, MaxChannels + 1) are pinned explicitly:
 * out of range is not an error, the setter returns false and the getter answers
 * with a fixed fallback value.
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <type_traits>

#include "data_lib/network_header.h"
#include "settings_lib/stream_settings.h"

namespace {

using AC_DC = CStreamSettings::AC_DC;
using ADCCaptureTime = CStreamSettings::ADCCaptureTime;
using Attenuator = CStreamSettings::Attenuator;
using DACGain = CStreamSettings::DACGain;
using DACPassMode = CStreamSettings::DACPassMode;
using DACRepeat = CStreamSettings::DACRepeat;
using DataFormat = CStreamSettings::DataFormat;
using DataType = CStreamSettings::DataType;
using PassMode = CStreamSettings::PassMode;
using Resolution = CStreamSettings::Resolution;
using State = CStreamSettings::State;
using TriggerEdge = CStreamSettings::TriggerEdge;
using EventSource = CStreamSettings::EventSource;

// Fills every field with a value different from its default, so that a missed
// field in copy() or resetDefault() shows up.
//
// Takes an out parameter rather than returning by value: CStreamSettings
// deletes its move constructor, so `return s;` from a factory is ill-formed.
// See CannotBeMovedOnlyCopied.
auto FillFullySetSettings(CStreamSettings& s) -> void {
    s.setADCSamples(4096);
    s.setADCFormat(DataFormat::TDMS);
    s.setADCType(DataType::VOLT);
    s.setADCCaptureTime(ADCCaptureTime::OFF);
    s.setADCPassMode(PassMode::FILE);
    s.setADCResolution(Resolution::BIT_8);
    s.setADCDecimation(64);
    s.setADCCalibration(State::OFF);
    for (uint8_t ch = 1; ch <= 4; ch++) {
        s.setADCChannels(ch, State::ON);
        s.setADCAttenuator(ch, Attenuator::A_1_20);
        s.setADCAC_DC(ch, AC_DC::AC);
    }
    s.setDACSpeed(1000000);
    s.setDACFile("/tmp/some.wav");
    s.setDACFileType(DataFormat::TDMS);
    s.setDACPassMode(DACPassMode::DAC_FILE);
    s.setDACRepeat(DACRepeat::DAC_REP_INF);
    s.setDACRepeatCount(7);
    s.setDACGain(1, DACGain::X5);
    s.setDACGain(2, DACGain::X5);
    s.setMemoryBlockSize(0x20000);
    s.setADCSize(111);
    s.setDACSize(222);
    s.setGPIOSize(333);
    s.setADCTriggerMode(State::ON);
    s.setADCTriggerSources(0x21);
    s.setADCTriggerLevelLow(-1000);
    s.setADCTriggerLevelHigh(2000);
    s.setADCTriggerEdge(TriggerEdge::FALLING);
    s.setADCEventSource(EventSource::GEN_A);
    s.setADCTriggerBlocks(5);
    s.setDACTriggerMode(1, State::ON);
    s.setDACTriggerMode(2, State::ON);
    s.setDACTriggerSources(0x04);
    s.setDACEventSource(EventSource::OSC1);
    s.setExtTrigConditioner(State::ON);
    s.setExtTrigEdge(TriggerEdge::FALLING);
    s.setExtTrigDebounce(1000);
    s.setExtTrigDio0(State::OFF);
    s.setGPIOInStreaming(State::ON);
    s.setGPIOOutStreaming(State::ON);
    s.setGPIOOutputsP(0x0F);
    s.setGPIOOutputsN(0xF0);
    s.setGPIOPolarity(0x0101);
    s.setGPIODecimation(4);
    s.setGPIORLE(State::ON);
    s.setGPIOOutStep(3);
    s.setGPIOPatternMask(0x00FF);
    s.setGPIOPatternValue(0x0055);
    s.setGPIOPatternPosEdge(0x0001);
    s.setGPIOPatternNegEdge(0x0002);
    s.setGPIOTriggerSources(0x10);
    s.setGPIOEventSource(EventSource::LA);
    s.setGPIOInTriggerMode(State::ON);
    s.setGPIOOutTriggerMode(State::ON);
    s.setGPIOTriggerBlocks(7);
}

auto SameSettings(const CStreamSettings& a, const CStreamSettings& b) -> ::testing::AssertionResult {
    if (a.toJson() != b.toJson()) {
        return ::testing::AssertionFailure() << "settings differ\n--- a ---\n" << a.toJson() << "\n--- b ---\n" << b.toJson();
    }
    return ::testing::AssertionSuccess();
}

}  // namespace

TEST(StreamSettings, TheDefaultsAreTheOnesDeclaredInTheHeader) {
    const CStreamSettings s;

    EXPECT_EQ(s.getADCPassMode(), PassMode::NET);
    EXPECT_EQ(s.getADCSamples(), 0u);
    EXPECT_EQ(s.getADCFormat(), DataFormat::BIN);
    EXPECT_EQ(s.getADCType(), DataType::RAW);
    EXPECT_EQ(s.getADCResolution(), Resolution::BIT_16);
    EXPECT_EQ(s.getADCDecimation(), 1u);
    EXPECT_EQ(s.getADCCalibration(), State::ON);
    EXPECT_EQ(s.getADCCaptureTime(), ADCCaptureTime::ON);
    for (uint8_t ch = 1; ch <= 4; ch++) {
        EXPECT_EQ(s.getADCChannels(ch), State::OFF) << "channel " << static_cast<int>(ch);
        EXPECT_EQ(s.getADCAttenuator(ch), Attenuator::A_1_1) << "channel " << static_cast<int>(ch);
        EXPECT_EQ(s.getADCAC_DC(ch), AC_DC::DC) << "channel " << static_cast<int>(ch);
    }

    EXPECT_EQ(s.getDACFile(), "");
    EXPECT_EQ(s.getDACFileType(), DataFormat::WAV);
    EXPECT_EQ(s.getDACPassMode(), DACPassMode::DAC_NET);
    EXPECT_EQ(s.getDACRepeat(), DACRepeat::DAC_REP_OFF);
    EXPECT_EQ(s.getDACRepeatCount(), 0u);
    EXPECT_EQ(s.getDACSpeed(), 125000000u);
    EXPECT_EQ(s.getDACGain(1), DACGain::X1);
    EXPECT_EQ(s.getDACGain(2), DACGain::X1);

    EXPECT_EQ(s.getMemoryBlockSize(), 0x10000u);
    const uint32_t expectedRegion = (0x10000u + DataLib::sizeHeader()) * 12u;
    EXPECT_EQ(s.getADCSize(), expectedRegion);
    EXPECT_EQ(s.getDACSize(), expectedRegion);
    EXPECT_EQ(s.getGPIOSize(), expectedRegion);

    // The trigger settings default to what the blocks do without them: no trigger, own events.
    EXPECT_EQ(s.getADCTriggerMode(), State::OFF);
    EXPECT_EQ(s.getADCTriggerSources(), 0u);
    EXPECT_EQ(s.getADCTriggerLevelLow(), -4);
    EXPECT_EQ(s.getADCTriggerLevelHigh(), 4);
    EXPECT_EQ(s.getADCTriggerEdge(), TriggerEdge::RISING);
    EXPECT_EQ(s.getADCEventSource(), EventSource::OWN);
    EXPECT_EQ(s.getADCTriggerBlocks(), 0u);
    EXPECT_EQ(s.getDACTriggerMode(1), State::OFF);
    EXPECT_EQ(s.getDACTriggerMode(2), State::OFF);
    EXPECT_EQ(s.getDACTriggerSources(), 0u);
    EXPECT_EQ(s.getDACEventSource(), EventSource::OWN);

    EXPECT_EQ(s.getExtTrigConditioner(), State::OFF);
    EXPECT_EQ(s.getExtTrigEdge(), TriggerEdge::RISING);
    EXPECT_EQ(s.getExtTrigDebounce(), 0u);
    EXPECT_EQ(s.getExtTrigDio0(), State::ON);

    EXPECT_EQ(s.getGPIOInStreaming(), State::OFF);
    EXPECT_EQ(s.getGPIOOutStreaming(), State::OFF);
    EXPECT_EQ(s.getGPIOOutputsP(), 0u);
    EXPECT_EQ(s.getGPIOOutputsN(), 0u);
    EXPECT_EQ(s.getGPIOPolarity(), 0u);
    EXPECT_EQ(s.getGPIODecimation(), 1u);
    EXPECT_EQ(s.getGPIORLE(), State::OFF);
    EXPECT_EQ(s.getGPIOOutStep(), 1u);
    EXPECT_EQ(s.getGPIOPatternMask(), 0u);
    EXPECT_EQ(s.getGPIOPatternValue(), 0u);
    EXPECT_EQ(s.getGPIOPatternPosEdge(), 0u);
    EXPECT_EQ(s.getGPIOPatternNegEdge(), 0u);
    EXPECT_EQ(s.getGPIOTriggerSources(), 0u);
    EXPECT_EQ(s.getGPIOEventSource(), EventSource::OWN);
    EXPECT_EQ(s.getGPIOInTriggerMode(), State::OFF);
    EXPECT_EQ(s.getGPIOOutTriggerMode(), State::OFF);
    EXPECT_EQ(s.getGPIOTriggerBlocks(), 0u);
}

// The range of every checked setter, at both bounds; a rejected value is not stored.
TEST(StreamSettings, TheTriggerAndGpioSettersCheckTheirRange) {
    CStreamSettings s;

    EXPECT_TRUE(s.setADCTriggerSources(0x3F));
    EXPECT_FALSE(s.setADCTriggerSources(0x40));
    EXPECT_EQ(s.getADCTriggerSources(), 0x3Fu);
    EXPECT_TRUE(s.setDACTriggerSources(0x3F));
    EXPECT_FALSE(s.setDACTriggerSources(0x80));
    EXPECT_TRUE(s.setGPIOTriggerSources(0x3F));
    EXPECT_FALSE(s.setGPIOTriggerSources(0x100));

    EXPECT_TRUE(s.setADCTriggerLevelLow(-32768));
    EXPECT_FALSE(s.setADCTriggerLevelLow(-32769));
    EXPECT_EQ(s.getADCTriggerLevelLow(), -32768);
    EXPECT_TRUE(s.setADCTriggerLevelHigh(32767));
    EXPECT_FALSE(s.setADCTriggerLevelHigh(32768));
    EXPECT_EQ(s.getADCTriggerLevelHigh(), 32767);
    // The order of the two levels is left to the consumer, so they can be set one by one.
    EXPECT_TRUE(s.setADCTriggerLevelLow(100));
    EXPECT_TRUE(s.setADCTriggerLevelHigh(-100));

    EXPECT_TRUE(s.setADCTriggerBlocks(0xFFFF));
    EXPECT_FALSE(s.setADCTriggerBlocks(0x10000));
    EXPECT_TRUE(s.setGPIOTriggerBlocks(0xFFFF));
    EXPECT_FALSE(s.setGPIOTriggerBlocks(0x10000));
    EXPECT_TRUE(s.setExtTrigDebounce(0xFFFF));
    EXPECT_FALSE(s.setExtTrigDebounce(0x10000));
    EXPECT_EQ(s.getExtTrigDebounce(), 0xFFFFu);

    EXPECT_TRUE(s.setGPIOOutputsP(0xFF));
    EXPECT_FALSE(s.setGPIOOutputsP(0x100));
    EXPECT_TRUE(s.setGPIOOutputsN(0xFF));
    EXPECT_FALSE(s.setGPIOOutputsN(0x100));
    for (auto setter : {&CStreamSettings::setGPIOPolarity, &CStreamSettings::setGPIOPatternMask, &CStreamSettings::setGPIOPatternValue,
                        &CStreamSettings::setGPIOPatternPosEdge, &CStreamSettings::setGPIOPatternNegEdge}) {
        EXPECT_TRUE((s.*setter)(0xFFFF));
        EXPECT_FALSE((s.*setter)(0x10000));
    }

    EXPECT_TRUE(s.setGPIODecimation(131072));
    EXPECT_FALSE(s.setGPIODecimation(131073));
    EXPECT_FALSE(s.setGPIODecimation(0));
    EXPECT_EQ(s.getGPIODecimation(), 131072u);
    EXPECT_FALSE(s.setGPIOOutStep(0));
    EXPECT_EQ(s.getGPIOOutStep(), 1u);
    EXPECT_TRUE(s.setGPIOOutStep(0xFFFFFFFFu));

    EXPECT_TRUE(s.setDACTriggerMode(2, State::ON));
    EXPECT_EQ(s.getDACTriggerMode(1), State::OFF);
    EXPECT_FALSE(s.setDACTriggerMode(0, State::ON));
    EXPECT_FALSE(s.setDACTriggerMode(3, State::ON));
    EXPECT_EQ(s.getDACTriggerMode(3), State::OFF);
}

// The event source enum carries the uio_lib EVENT_SRC_* numbers, OWN is not a register value.
TEST(StreamSettings, TheEventSourceValuesMatchUioLib) {
    EXPECT_EQ(static_cast<int>(EventSource::OWN), -1);
    EXPECT_EQ(static_cast<int>(EventSource::GEN_A), 0);
    EXPECT_EQ(static_cast<int>(EventSource::GEN_B), 1);
    EXPECT_EQ(static_cast<int>(EventSource::OSC1), 2);
    EXPECT_EQ(static_cast<int>(EventSource::OSC2), 3);
    EXPECT_EQ(static_cast<int>(EventSource::LA), 4);
    EXPECT_EQ(CStreamSettings::TRIGGER_SOURCES_ALL, 0x3Fu);
}

TEST(StreamSettings, AdcChannelAccessorsAreOneBasedAndRangeChecked) {
    CStreamSettings s;

    for (uint8_t ch = 1; ch <= 4; ch++) {
        EXPECT_TRUE(s.setADCChannels(ch, State::ON)) << "channel " << static_cast<int>(ch);
        EXPECT_TRUE(s.setADCAttenuator(ch, Attenuator::A_1_20));
        EXPECT_TRUE(s.setADCAC_DC(ch, AC_DC::AC));
    }
    for (uint8_t ch = 1; ch <= 4; ch++) {
        EXPECT_EQ(s.getADCChannels(ch), State::ON);
        EXPECT_EQ(s.getADCAttenuator(ch), Attenuator::A_1_20);
        EXPECT_EQ(s.getADCAC_DC(ch), AC_DC::AC);
    }

    // Channel 0 and channel 5 are rejected rather than clamped or wrapped.
    for (uint8_t ch : {uint8_t{0}, uint8_t{5}, uint8_t{255}}) {
        EXPECT_FALSE(s.setADCChannels(ch, State::OFF)) << "channel " << static_cast<int>(ch);
        EXPECT_FALSE(s.setADCAttenuator(ch, Attenuator::A_1_1));
        EXPECT_FALSE(s.setADCAC_DC(ch, AC_DC::DC));
        // The getters answer with a fixed fallback, not with channel 1's value.
        EXPECT_EQ(s.getADCChannels(ch), State::OFF);
        EXPECT_EQ(s.getADCAttenuator(ch), Attenuator::A_1_1);
        EXPECT_EQ(s.getADCAC_DC(ch), AC_DC::DC);
    }
    // The rejected writes left the real channels alone.
    EXPECT_EQ(s.getADCChannels(1), State::ON);
    EXPECT_EQ(s.getADCAttenuator(4), Attenuator::A_1_20);
}

TEST(StreamSettings, EachAdcChannelIsIndependent) {
    CStreamSettings s;
    ASSERT_TRUE(s.setADCChannels(2, State::ON));
    ASSERT_TRUE(s.setADCAttenuator(3, Attenuator::A_1_20));
    ASSERT_TRUE(s.setADCAC_DC(4, AC_DC::AC));

    EXPECT_EQ(s.getADCChannels(1), State::OFF);
    EXPECT_EQ(s.getADCChannels(2), State::ON);
    EXPECT_EQ(s.getADCChannels(3), State::OFF);
    EXPECT_EQ(s.getADCAttenuator(2), Attenuator::A_1_1);
    EXPECT_EQ(s.getADCAttenuator(3), Attenuator::A_1_20);
    EXPECT_EQ(s.getADCAC_DC(3), AC_DC::DC);
    EXPECT_EQ(s.getADCAC_DC(4), AC_DC::AC);
}

TEST(StreamSettings, DacGainHasTwoChannelsOnly) {
    CStreamSettings s;
    EXPECT_TRUE(s.setDACGain(1, DACGain::X5));
    EXPECT_TRUE(s.setDACGain(2, DACGain::X5));
    EXPECT_FALSE(s.setDACGain(0, DACGain::X1));
    EXPECT_FALSE(s.setDACGain(3, DACGain::X1));
    EXPECT_FALSE(s.setDACGain(4, DACGain::X1));

    EXPECT_EQ(s.getDACGain(1), DACGain::X5);
    EXPECT_EQ(s.getDACGain(2), DACGain::X5);
    EXPECT_EQ(s.getDACGain(3), DACGain::X1) << "out of range reads the fallback";
}

TEST(StreamSettings, DecimationIsOneToSixtyFiveThousandFiveHundredThirtySix) {
    CStreamSettings s;
    EXPECT_TRUE(s.setADCDecimation(1));
    EXPECT_EQ(s.getADCDecimation(), 1u);
    EXPECT_TRUE(s.setADCDecimation(65536)) << "the upper bound is inclusive";
    EXPECT_EQ(s.getADCDecimation(), 65536u);

    EXPECT_FALSE(s.setADCDecimation(65537));
    EXPECT_EQ(s.getADCDecimation(), 65536u) << "a rejected value must not be stored";

    // Zero is not a usable decimation factor and is rejected like any other
    // out-of-range value; getHelp() advertises the same 1-65536.
    EXPECT_FALSE(s.setADCDecimation(0));
    EXPECT_EQ(s.getADCDecimation(), 65536u);
    EXPECT_NE(CStreamSettings::getHelp().find("1-65536"), std::string::npos);
}

// BIN is a valid ADC recording format but not a valid DAC playback format, and
// the setter enforces that by refusing the write. Callers that ignore the
// return value silently keep the previous type - parseJson is one of them.
TEST(StreamSettings, TheDacFileTypeRefusesBin) {
    CStreamSettings s;
    ASSERT_TRUE(s.setDACFileType(DataFormat::TDMS));
    EXPECT_EQ(s.getDACFileType(), DataFormat::TDMS);

    EXPECT_FALSE(s.setDACFileType(DataFormat::BIN));
    EXPECT_EQ(s.getDACFileType(), DataFormat::TDMS) << "the rejected write must leave the old value";

    EXPECT_TRUE(s.setDACFileType(DataFormat::WAV));
    EXPECT_EQ(s.getDACFileType(), DataFormat::WAV);
}

TEST(StreamSettings, ScalarSettersStoreExactlyWhatTheyAreGiven) {
    CStreamSettings s;
    s.setADCSamples(0xFFFFFFFFFFULL);
    EXPECT_EQ(s.getADCSamples(), 0xFFFFFFFFFFULL) << "samples is 64-bit and must not be truncated";

    EXPECT_TRUE(s.setDACSpeed(0));
    EXPECT_EQ(s.getDACSpeed(), 0u);
    EXPECT_TRUE(s.setDACSpeed(0xFFFFFFFFu));
    EXPECT_EQ(s.getDACSpeed(), 0xFFFFFFFFu);

    s.setDACRepeatCount(0xFFFFFFFFu);
    EXPECT_EQ(s.getDACRepeatCount(), 0xFFFFFFFFu);

    s.setDACFile("/home/redpitaya/streaming_files/dac/x.wav");
    EXPECT_EQ(s.getDACFile(), "/home/redpitaya/streaming_files/dac/x.wav");
    s.setDACFile("");
    EXPECT_EQ(s.getDACFile(), "");

    s.setMemoryBlockSize(1);
    s.setADCSize(2);
    s.setDACSize(3);
    s.setGPIOSize(4);
    EXPECT_EQ(s.getMemoryBlockSize(), 1u);
    EXPECT_EQ(s.getADCSize(), 2u);
    EXPECT_EQ(s.getDACSize(), 3u);
    EXPECT_EQ(s.getGPIOSize(), 4u);
}

TEST(StreamSettings, ResetDefaultRestoresEveryGroup) {
    CStreamSettings s;
    FillFullySetSettings(s);
    ASSERT_FALSE(SameSettings(s, CStreamSettings()));

    s.resetDefault();
    EXPECT_TRUE(SameSettings(s, CStreamSettings()));
}

TEST(StreamSettings, CopyConstructionAssignmentAndCopyAllCarryEveryField) {
    CStreamSettings source;
    FillFullySetSettings(source);

    const CStreamSettings constructed(source);
    EXPECT_TRUE(SameSettings(constructed, source));

    CStreamSettings assigned;
    assigned = source;
    EXPECT_TRUE(SameSettings(assigned, source));

    CStreamSettings copied;
    copied.copy(source);
    EXPECT_TRUE(SameSettings(copied, source));

    // The copies are independent of the source.
    CStreamSettings mutated(source);
    mutated.setADCSamples(1);
    EXPECT_EQ(source.getADCSamples(), 4096u);
    EXPECT_EQ(mutated.getADCSamples(), 1u);
}

// The move constructor and move assignment are declared private AND deleted, so
// a CStreamSettings cannot be returned by value from a factory, stored in a
// vector that reallocates, or std::move'd into a member - every one of those is
// a compile error rather than a silent copy. Copying is the only way to pass one
// around, and copying is not cheap (three structs plus a std::string). Pinned so
// that the restriction is visible without discovering it through a build break.
TEST(StreamSettings, CannotBeMovedOnlyCopied) {
    static_assert(std::is_copy_constructible_v<CStreamSettings>);
    static_assert(std::is_copy_assignable_v<CStreamSettings>);
    static_assert(!std::is_move_constructible_v<CStreamSettings>);
    static_assert(!std::is_move_assignable_v<CStreamSettings>);
    SUCCEED();
}

TEST(StreamSettings, SelfAssignmentKeepsTheValue) {
    CStreamSettings s;
    FillFullySetSettings(s);
    const std::string before = s.toJson();
    CStreamSettings& alias = s;
    s = alias;
    EXPECT_EQ(s.toJson(), before);
}

// toString() is the human dump; it is not parsed back anywhere, so only the
// facts a reader relies on are pinned - plus the two current oddities, so that
// fixing them is a deliberate act and not a silent output change.
TEST(StreamSettings, ToStringReportsEveryGroup) {
    CStreamSettings s;
    s.setADCSamples(0);
    const std::string dump = s.toString();

    EXPECT_NE(dump.find("ADC streaming"), std::string::npos);
    EXPECT_NE(dump.find("DAC streaming"), std::string::npos);
    EXPECT_NE(dump.find("Memory Manager"), std::string::npos);
    EXPECT_NE(dump.find("External trigger"), std::string::npos);
    EXPECT_NE(dump.find("GPIO streaming"), std::string::npos);
    EXPECT_NE(dump.find("Trigger blocks:\t\tContinuous"), std::string::npos) << "0 trigger blocks prints as Continuous";
    EXPECT_NE(dump.find("Samples:\t\tUnlimited"), std::string::npos) << "0 samples prints as Unlimited";

    s.setADCSamples(10);
    EXPECT_NE(s.toString().find("Samples:\t\t10"), std::string::npos);

    // The resolution is printed through to_string(), which now carries the
    // declared display text whole instead of stopping at the space.
    EXPECT_NE(dump.find("Resolution:\t\t16 Bit\n"), std::string::npos);

    // Two DAC channels exist, so exactly two gain lines are printed.
    EXPECT_NE(dump.find("Ch 1 Gain"), std::string::npos);
    EXPECT_NE(dump.find("Ch 2 Gain"), std::string::npos);
    EXPECT_EQ(dump.find("Ch 3 Gain"), std::string::npos);
    EXPECT_EQ(dump.find("Ch 4 Gain"), std::string::npos);
}

TEST(StreamSettings, GetHelpNamesEveryKeyTheKeyValueInterfaceAccepts) {
    const std::string help = CStreamSettings::getHelp();
    for (const char* key : {"format_sd", "data_type_sd", "samples_limit_sd", "adc_pass_mode", "resolution", "adc_decimation", "use_calib",
                            "adc_capture_time", "dac_rate", "file_sd", "file_type_sd", "dac_pass_mode", "repeat", "repeatCount", "block_size",
                            "adc_size", "dac_size", "gpio_size", "adc_trigger_mode", "adc_trigger_sources", "adc_trigger_level_low",
                            "adc_trigger_level_high", "adc_trigger_edge", "adc_event_source", "adc_trigger_blocks", "dac_trigger_sources",
                            "dac_event_source", "ext_trig_conditioner", "ext_trig_edge", "ext_trig_debounce", "ext_trig_dio0",
                            "gpio_in_streaming", "gpio_out_streaming", "gpio_outputs_p", "gpio_outputs_n", "gpio_polarity", "gpio_decimation",
                            "gpio_rle", "gpio_out_step", "gpio_pattern_mask", "gpio_pattern_value", "gpio_pattern_pos_edge",
                            "gpio_pattern_neg_edge", "gpio_trigger_sources", "gpio_event_source", "gpio_in_trigger_mode",
                            "gpio_out_trigger_mode", "gpio_trigger_blocks"}) {
        EXPECT_NE(help.find(key), std::string::npos) << "missing key " << key;
    }
    for (int i = 1; i <= 4; i++) {
        EXPECT_NE(help.find("channel_state_" + std::to_string(i)), std::string::npos);
        EXPECT_NE(help.find("channel_attenuator_" + std::to_string(i)), std::string::npos);
        EXPECT_NE(help.find("channel_ac_dc_" + std::to_string(i)), std::string::npos);
    }
    for (int i = 1; i <= 2; i++) {
        EXPECT_NE(help.find("channel_gain_" + std::to_string(i)), std::string::npos);
        EXPECT_NE(help.find("dac_trigger_mode_" + std::to_string(i)), std::string::npos);
    }
    // The listed alternatives come from names(), which is what setValue parses.
    EXPECT_NE(help.find("WAV,TDMS,BIN"), std::string::npos);
    EXPECT_NE(help.find("DAC_REP_OFF,DAC_REP_INF,DAC_REP_ON"), std::string::npos);
    EXPECT_NE(help.find("OWN,GEN_A,GEN_B,OSC1,OSC2,LA"), std::string::npos);
    EXPECT_NE(help.find("RISING,FALLING"), std::string::npos);
}

// getDACDirPath is global process state guarded by a preprocessor switch: the
// RedPitaya build creates and returns the configured directory, every other
// build ignores it and answers ".". Both are accepted here so the test means
// the same thing in either configuration; what is pinned unconditionally is
// that getDACFiles() lists exactly that directory.
TEST(StreamSettings, TheDacDirectoryListingMatchesTheDacDirectory) {
    const std::string original = CStreamSettings::getDACDirPath();
    ASSERT_FALSE(original.empty());

    std::error_code ec;
    const std::filesystem::path scratch = std::filesystem::temp_directory_path() / "settings_lib_dac_dir_test";
    std::filesystem::create_directories(scratch, ec);
    { std::ofstream(scratch / "a.wav"); }
    { std::ofstream(scratch / "b.tdms"); }

    std::string requested = scratch.string();
    CStreamSettings::setDACDirPath(requested);
    const std::string effective = CStreamSettings::getDACDirPath();
    EXPECT_TRUE(effective == requested || effective == ".") << "unexpected DAC dir: " << effective;

    std::string expected;
    for (const auto& entry : std::filesystem::directory_iterator(effective)) {
        expected += entry.path().filename().generic_string() + "\n";
    }
    EXPECT_EQ(CStreamSettings::getDACFiles(), expected);

    std::string restore = original;
    CStreamSettings::setDACDirPath(restore);
    std::filesystem::remove_all(scratch, ec);
}
