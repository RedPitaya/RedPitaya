/**
 * The string key/value interface: setValue / getValue.
 *
 * This is the surface the command line and the network config layer drive, so
 * the contract that matters is the round trip getValue(k) -> setValue(k, v) and
 * the failure mode for bad input: setValue returns false and changes nothing,
 * getValue returns the literal string "ERROR". Neither ever throws - every
 * conversion is wrapped in a catch-all inside the library.
 */

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include "json/json.h"
#include <vector>

#include "settings_lib/stream_settings.h"

namespace {

// Every key the interface understands, with a value that differs from the
// default so a lost write is visible.
struct KeyValue {
    const char* key;
    const char* value;
};

const std::vector<KeyValue> kAllKeys = {
    {"format_sd", "TDMS"},
    {"data_type_sd", "VOLT"},
    {"adc_capture_time", "OFF"},
    {"tdms_time_column", "ON"},
    {"samples_limit_sd", "123456789012"},
    {"limit_time_sd", "90.5"},
    {"split_size_sd", "1000000000"},
    {"adc_pass_mode", "FILE"},
    {"resolution", "BIT_8"},
    {"adc_decimation", "8"},
    {"use_calib", "OFF"},
    {"channel_state_1", "ON"},
    {"channel_state_2", "ON"},
    {"channel_state_3", "ON"},
    {"channel_state_4", "ON"},
    {"channel_attenuator_1", "A_1_20"},
    {"channel_attenuator_2", "A_1_20"},
    {"channel_attenuator_3", "A_1_20"},
    {"channel_attenuator_4", "A_1_20"},
    {"channel_ac_dc_1", "AC"},
    {"channel_ac_dc_2", "AC"},
    {"channel_ac_dc_3", "AC"},
    {"channel_ac_dc_4", "AC"},
    {"dac_rate", "1000000"},
    {"file_sd", "generated.wav"},
    {"file_type_sd", "TDMS"},
    {"dac_pass_mode", "DAC_FILE"},
    {"repeat", "DAC_REP_INF"},
    {"repeatCount", "9"},
    {"channel_gain_1", "X5"},
    {"channel_gain_2", "X5"},
    {"block_size", "131072"},
    {"adc_size", "1000"},
    {"dac_size", "2000"},
    {"gpio_in_size", "3000"},
    {"gpio_out_size", "4000"},
    {"adc_trigger_mode", "ON"},
    {"adc_trigger_sources", "33"},
    {"adc_trigger_level_low", "-1000"},
    {"adc_trigger_level_high", "2000"},
    {"adc_trigger_edge", "FALLING"},
    {"adc_event_source", "GEN_B"},
    {"adc_trigger_blocks", "10"},
    {"dac_trigger_mode_1", "ON"},
    {"dac_trigger_mode_2", "ON"},
    {"dac_trigger_sources", "4"},
    {"dac_event_source", "OSC2"},
    {"ext_trig_conditioner", "ON"},
    {"ext_trig_edge", "FALLING"},
    {"ext_trig_debounce", "200"},
    {"ext_trig_dio0", "OFF"},
    {"gpio_outputs_p", "255"},
    {"gpio_outputs_n", "1"},
    {"gpio_polarity", "65535"},
    {"gpio_decimation", "131072"},
    {"gpio_rle", "ON"},
    {"gpio_out_step", "2"},
    {"gpio_pattern_mask", "3"},
    {"gpio_pattern_value", "1"},
    {"gpio_pattern_pos_edge", "2"},
    {"gpio_pattern_neg_edge", "4"},
    {"gpio_trigger_sources", "63"},
    {"gpio_event_source", "LA"},
    {"gpio_in_trigger_mode", "ON"},
    {"gpio_out_trigger_mode", "ON"},
    {"gpio_trigger_blocks", "65535"},
    {"gpio_in_pass_mode", "FILE"},
    {"gpio_in_format_sd", "VCD"},
    {"gpio_in_samples_limit_sd", "123456789012"},
    {"gpio_in_limit_size_sd", "5000000"},
    {"gpio_in_split_time_sd", "60"},
    {"gpio_out_pass_mode", "FILE"},
    {"gpio_out_file_sd", "pattern.bin"},
    {"gpio_out_repeat", "GPIO_REP_ON"},
    {"gpio_out_repeat_count", "17"},
    {"gpio_out_play_gaps", "ON"},
    {"dac_play_gaps", "ON"},
};

}  // namespace

TEST(StreamSettingsKeys, EveryKeyAcceptsAValueAndReadsItBack) {
    CStreamSettings s;
    for (const auto& kv : kAllKeys) {
        EXPECT_TRUE(s.setValue(kv.key, kv.value)) << "setValue rejected " << kv.key;
        EXPECT_EQ(s.getValue(kv.key), kv.value) << "getValue disagreed for " << kv.key;
    }
}

TEST(StreamSettingsKeys, GetValueThenSetValueIsAnIdentityForEveryKey) {
    CStreamSettings source;
    for (const auto& kv : kAllKeys) {
        ASSERT_TRUE(source.setValue(kv.key, kv.value)) << kv.key;
    }

    CStreamSettings target;
    for (const auto& kv : kAllKeys) {
        const std::string carried = source.getValue(kv.key);
        ASSERT_NE(carried, "ERROR") << kv.key;
        EXPECT_TRUE(target.setValue(kv.key, carried)) << kv.key;
    }
    EXPECT_EQ(target.toJson(), source.toJson());
}

TEST(StreamSettingsKeys, TheDefaultsSurviveTheSameRoundTrip) {
    CStreamSettings source;  // not const: getValue() is a non-const member
    CStreamSettings target;
    // Move the target away from the defaults first, so that a key whose value
    // is silently dropped shows up instead of matching by accident.
    for (const auto& kv : kAllKeys) {
        ASSERT_TRUE(target.setValue(kv.key, kv.value)) << kv.key;
    }
    for (const auto& kv : kAllKeys) {
        const std::string carried = source.getValue(kv.key);
        ASSERT_NE(carried, "ERROR") << kv.key;
        EXPECT_TRUE(target.setValue(kv.key, carried)) << kv.key;
    }
    EXPECT_EQ(target.toJson(), source.toJson());
}

TEST(StreamSettingsKeys, AnUnknownKeyIsRejectedWithoutThrowing) {
    CStreamSettings s;
    const std::string before = s.toJson();

    EXPECT_FALSE(s.setValue("", "1"));
    EXPECT_FALSE(s.setValue("no_such_key", "1"));
    EXPECT_FALSE(s.setValue("channel_state_0", "ON")) << "channels are 1..4";
    EXPECT_FALSE(s.setValue("channel_state_5", "ON"));
    EXPECT_FALSE(s.setValue("channel_gain_3", "X5")) << "DAC gain is 1..2";
    EXPECT_FALSE(s.setValue("FORMAT_SD", "TDMS")) << "keys are case sensitive";

    EXPECT_EQ(s.getValue(""), "ERROR");
    EXPECT_EQ(s.getValue("no_such_key"), "ERROR");
    EXPECT_EQ(s.getValue("channel_state_5"), "ERROR");
    EXPECT_EQ(s.getValue("channel_gain_3"), "ERROR");

    EXPECT_EQ(s.toJson(), before) << "a rejected key must not change anything";
}

TEST(StreamSettingsKeys, AnUnparsableValueIsRejectedWithoutThrowing) {
    CStreamSettings s;
    const std::string before = s.toJson();

    // Enum keys only accept the enumerator identifier.
    EXPECT_FALSE(s.setValue("format_sd", "wav")) << "the display string is not accepted";
    EXPECT_FALSE(s.setValue("format_sd", ""));
    EXPECT_FALSE(s.setValue("resolution", "BIT_32"));
    EXPECT_FALSE(s.setValue("repeat", "Infinity"));
    EXPECT_FALSE(s.setValue("channel_state_1", "on"));

    // Numeric keys reject anything that is not all digits.
    EXPECT_FALSE(s.setValue("adc_decimation", ""));
    EXPECT_FALSE(s.setValue("adc_decimation", "0")) << "0 is not a usable decimation factor";
    EXPECT_FALSE(s.setValue("adc_decimation", "-1"));
    EXPECT_FALSE(s.setValue("adc_decimation", "12abc"));
    EXPECT_FALSE(s.setValue("adc_decimation", " 12")) << "leading whitespace is not stripped";
    EXPECT_FALSE(s.setValue("adc_decimation", "1.5"));
    EXPECT_FALSE(s.setValue("samples_limit_sd", "0x10"));
    EXPECT_FALSE(s.setValue("dac_rate", "+7"));

    EXPECT_EQ(s.toJson(), before);
}

// adc_decimation is the one numeric key with a range check, and setValue passes
// the setter's own false straight through.
TEST(StreamSettingsKeys, ARangeCheckedValueIsReportedByItsSetter) {
    CStreamSettings s;
    EXPECT_TRUE(s.setValue("adc_decimation", "65536"));
    EXPECT_EQ(s.getValue("adc_decimation"), "65536");
    EXPECT_FALSE(s.setValue("adc_decimation", "65537"));
    EXPECT_EQ(s.getValue("adc_decimation"), "65536") << "the rejected value must not be stored";

    // file_type_sd takes BIN too: the BIN stream file plays on the DAC
    EXPECT_TRUE(s.setValue("file_type_sd", "BIN"));
    EXPECT_EQ(s.getValue("file_type_sd"), "BIN");
    EXPECT_FALSE(s.setValue("file_type_sd", "MP3"));
}

// to_uint/to_uint64 check for overflow instead of wrapping, so a value too wide
// for the target type is bad input rather than a silently different number.
TEST(StreamSettingsKeys, NumericOverflowIsRejectedNotWrapped) {
    CStreamSettings s;
    const std::string rate = s.getValue("dac_rate");
    const std::string samples = s.getValue("samples_limit_sd");

    EXPECT_FALSE(s.setValue("dac_rate", "4294967296"));  // 2^32
    EXPECT_EQ(s.getValue("dac_rate"), rate) << "nothing stored";
    EXPECT_TRUE(s.setValue("dac_rate", "4294967295")) << "the widest value that does fit is still accepted";
    EXPECT_EQ(s.getValue("dac_rate"), "4294967295");

    EXPECT_FALSE(s.setValue("samples_limit_sd", "18446744073709551616"));  // 2^64
    EXPECT_EQ(s.getValue("samples_limit_sd"), samples);
    EXPECT_TRUE(s.setValue("samples_limit_sd", "18446744073709551615"));
    EXPECT_EQ(s.getValue("samples_limit_sd"), "18446744073709551615");

    EXPECT_FALSE(s.setValue("block_size", "99999999999999999999999999")) << "far past 64 bits, rejected in to_uint64";
}

// The trigger levels are the only signed keys: an optional minus, then digits only.
TEST(StreamSettingsKeys, TheTriggerLevelsTakeSignedValues) {
    CStreamSettings s;
    EXPECT_TRUE(s.setValue("adc_trigger_level_low", "-32768"));
    EXPECT_EQ(s.getValue("adc_trigger_level_low"), "-32768");
    EXPECT_TRUE(s.setValue("adc_trigger_level_high", "32767"));
    EXPECT_TRUE(s.setValue("adc_trigger_level_high", "-0"));
    EXPECT_EQ(s.getValue("adc_trigger_level_high"), "0");

    EXPECT_FALSE(s.setValue("adc_trigger_level_low", "-32769")) << "out of the ADC code range";
    EXPECT_FALSE(s.setValue("adc_trigger_level_high", "32768"));
    EXPECT_FALSE(s.setValue("adc_trigger_level_low", "-"));
    EXPECT_FALSE(s.setValue("adc_trigger_level_low", "--1"));
    EXPECT_FALSE(s.setValue("adc_trigger_level_low", "+1"));
    EXPECT_FALSE(s.setValue("adc_trigger_level_low", "-9999999999"));
    EXPECT_EQ(s.getValue("adc_trigger_level_low"), "-32768") << "nothing stored";

    EXPECT_FALSE(s.setValue("adc_trigger_blocks", "-1")) << "the other numeric keys stay unsigned";
    EXPECT_FALSE(s.setValue("gpio_decimation", "0"));
    EXPECT_FALSE(s.setValue("gpio_out_step", "0"));
    EXPECT_FALSE(s.setValue("adc_trigger_sources", "64"));
    EXPECT_FALSE(s.setValue("adc_event_source", "DAC A")) << "the display string is not accepted";
    EXPECT_FALSE(s.setValue("dac_trigger_mode_3", "ON"));
    EXPECT_EQ(s.getValue("dac_trigger_mode_3"), "ERROR");
}

TEST(StreamSettingsKeys, TheDACFileKeyTakesANameOnly) {
    CStreamSettings s;
    EXPECT_TRUE(s.setValue("file_sd", ""));
    EXPECT_EQ(s.getValue("file_sd"), "");
    EXPECT_TRUE(s.setValue("file_sd", "  a b.c.wav  "));
    EXPECT_EQ(s.getValue("file_sd"), "  a b.c.wav  ") << "no trimming";
    EXPECT_TRUE(s.setValue("file_sd", "..x.wav"));
    for (const char* bad : {"a b/c.wav", "/tmp/x.wav", "../x.wav", "..", ".", "dir\\x.wav"}) {
        EXPECT_FALSE(s.setValue("file_sd", bad)) << bad;
        EXPECT_EQ(s.getValue("file_sd"), "..x.wav") << bad;
    }
    EXPECT_FALSE(s.setValue("file_sd", std::string(256, 'a')));
    EXPECT_TRUE(s.setValue("file_sd", std::string(255, 'a')));
    EXPECT_FALSE(s.setDACFile(std::string("a\0b", 3)));
    // the page lists the files separated by '*' and line ends (streaming_improvements 1.9)
    for (const char* bad : {"a*b.wav", "a\nb.wav", "a\rb.wav", "a\tb.wav", "a\x7f.wav"})
        EXPECT_FALSE(s.setValue("file_sd", bad)) << bad;
}

TEST(StreamSettingsKeys, TheBlockSizeIsAMultipleOf128) {
    CStreamSettings s;
    const auto before = s.getMemoryBlockSize();
    for (const char* bad : {"0", "1", "127", "129", "65537", "268435584", "4294967168"}) {
        EXPECT_FALSE(s.setValue("block_size", bad)) << bad;
        EXPECT_EQ(s.getMemoryBlockSize(), before) << bad;
    }
    for (uint32_t good : {128u, 512u, 2048u, 65536u, 8388608u, CStreamSettings::MEMORY_BLOCK_MAX}) {
        EXPECT_TRUE(s.setValue("block_size", std::to_string(good))) << good;
        EXPECT_EQ(s.getMemoryBlockSize(), good);
    }
}

TEST(StreamSettingsKeys, EveryChannelKeyAddressesItsOwnChannel) {
    CStreamSettings s;
    ASSERT_TRUE(s.setValue("channel_state_3", "ON"));
    EXPECT_EQ(s.getValue("channel_state_1"), "OFF");
    EXPECT_EQ(s.getValue("channel_state_2"), "OFF");
    EXPECT_EQ(s.getValue("channel_state_3"), "ON");
    EXPECT_EQ(s.getValue("channel_state_4"), "OFF");

    ASSERT_TRUE(s.setValue("channel_gain_2", "X5"));
    EXPECT_EQ(s.getValue("channel_gain_1"), "X1");
    EXPECT_EQ(s.getValue("channel_gain_2"), "X5");
}

TEST(StreamSettingsKeys, TheGpioOutputFileIsANameOnly) {
    CStreamSettings s;
    EXPECT_TRUE(s.setValue("gpio_out_file_sd", "lines.bin"));
    for (const char* bad : {"../lines.bin", "/tmp/lines.bin", "dir/lines.bin", "..", "a*b.bin", "a\nb.bin"}) {
        EXPECT_FALSE(s.setValue("gpio_out_file_sd", bad)) << bad;
        EXPECT_EQ(s.getValue("gpio_out_file_sd"), "lines.bin") << bad;
    }
    EXPECT_FALSE(s.setValue("gpio_in_format_sd", "WAV")) << "the GPIO input has its own formats";
    EXPECT_FALSE(s.setValue("gpio_out_repeat", "DAC_REP_ON"));
}

// The level window in volts: with the unit VOLT the codes follow the volts, the edge and the
// range of the first input among the sources (1 V at 1:1, 20 V at 1:20), as the volts of the files.
TEST(StreamSettingsKeys, TheVoltsOfTheTriggerSetItsCodes) {
    CStreamSettings s;
    EXPECT_EQ(s.getValue("adc_trigger_level_unit"), "CODE");
    ASSERT_TRUE(s.setValue("adc_trigger_sources", "4"));  // CH1
    ASSERT_TRUE(s.setValue("adc_trigger_level_v", "0.25"));
    ASSERT_TRUE(s.setValue("adc_trigger_hysteresis_v", "0.05"));
    EXPECT_EQ(s.getValue("adc_trigger_level_low"), "-4") << "the codes as set while the unit is CODE";
    ASSERT_TRUE(s.setValue("adc_trigger_level_unit", "VOLT"));
    EXPECT_EQ(s.getValue("adc_trigger_level_low"), "6554");  // 0.20 V
    EXPECT_EQ(s.getValue("adc_trigger_level_high"), "8192");  // 0.25 V

    ASSERT_TRUE(s.setValue("adc_trigger_edge", "FALLING"));  // fires at the level coming down
    EXPECT_EQ(s.getValue("adc_trigger_level_low"), "8192");
    EXPECT_EQ(s.getValue("adc_trigger_level_high"), "9830");  // 0.30 V

    ASSERT_TRUE(s.setValue("adc_trigger_edge", "RISING"));
    ASSERT_TRUE(s.setValue("channel_attenuator_1", "A_1_20"));
    EXPECT_EQ(s.getValue("adc_trigger_level_low"), "328");
    EXPECT_EQ(s.getValue("adc_trigger_level_high"), "410");
    EXPECT_EQ(s.getValue("adc_trigger_level_v"), "0.25") << "the volts stay, the codes follow the range";

    ASSERT_TRUE(s.setValue("adc_trigger_sources", "8"));  // CH2 first when CH1 is not a source
    EXPECT_EQ(s.getValue("adc_trigger_level_high"), "8192");

    // back to the codes: the ones set
    ASSERT_TRUE(s.setValue("adc_trigger_level_unit", "CODE"));
    EXPECT_EQ(s.getValue("adc_trigger_level_low"), "-4");
    EXPECT_EQ(s.getValue("adc_trigger_level_high"), "4");
}

TEST(StreamSettingsKeys, TheVoltsOfTheTriggerAreChecked) {
    CStreamSettings s;
    for (const char* bad : {"21", "-20.5", "abc", "", "1e400", "nan", "0.1x"})
        EXPECT_FALSE(s.setValue("adc_trigger_level_v", bad)) << bad;
    for (const char* bad : {"-0.01", "41", "inf"})
        EXPECT_FALSE(s.setValue("adc_trigger_hysteresis_v", bad)) << bad;
    EXPECT_FALSE(s.setValue("adc_trigger_level_unit", "MV"));
    EXPECT_EQ(s.getValue("adc_trigger_level_unit"), "CODE") << "a refused value changes nothing";
    // codes beyond the range saturate
    ASSERT_TRUE(s.setValue("adc_trigger_sources", "4"));
    ASSERT_TRUE(s.setValue("adc_trigger_level_unit", "VOLT"));
    ASSERT_TRUE(s.setValue("adc_trigger_level_v", "5"));
    EXPECT_EQ(s.getValue("adc_trigger_level_high"), "32767");
}

// every key keeps its own value: any order of the keys gives the same settings
TEST(StreamSettingsKeys, TheVoltsOfTheTriggerSurviveTheJsonAndAnyKeyOrder) {
    CStreamSettings s;
    ASSERT_TRUE(s.setValue("adc_trigger_sources", "4"));
    ASSERT_TRUE(s.setValue("adc_trigger_level_unit", "VOLT"));
    ASSERT_TRUE(s.setValue("adc_trigger_level_v", "-0.125"));
    ASSERT_TRUE(s.setValue("adc_trigger_hysteresis_v", "0.0625"));
    CStreamSettings t;
    ASSERT_TRUE(t.parseJson(s.toJson()));
    EXPECT_EQ(t.getValue("adc_trigger_level_unit"), "VOLT");
    EXPECT_EQ(t.getValue("adc_trigger_level_v"), "-0.125");
    EXPECT_EQ(t.getValue("adc_trigger_hysteresis_v"), "0.0625");
    EXPECT_EQ(t.getValue("adc_trigger_level_low"), s.getValue("adc_trigger_level_low"));
    EXPECT_EQ(t.toJson(), s.toJson());
    const char* keys[] = {"adc_trigger_sources", "adc_trigger_level_low", "adc_trigger_level_high", "adc_trigger_level_unit", "adc_trigger_level_v",
                          "adc_trigger_hysteresis_v"};
    CStreamSettings forward, backward;
    for (auto* k : keys)
        ASSERT_TRUE(forward.setValue(k, s.getValue(k))) << k;
    for (int i = 5; i >= 0; i--)
        ASSERT_TRUE(backward.setValue(keys[i], s.getValue(keys[i]))) << keys[i];
    EXPECT_EQ(forward.toJson(), s.toJson());
    EXPECT_EQ(backward.toJson(), s.toJson());
}

// ---- the limits of a file and its parts (streaming_improvements 4.11)

TEST(StreamSettingsKeys, OneLimitAndOneSplitAtATime) {
    CStreamSettings s;
    ASSERT_TRUE(s.setValue("limit_time_sd", "600"));
    ASSERT_TRUE(s.setValue("limit_size_sd", "2000000000"));
    EXPECT_EQ(s.getValue("limit_time_sd"), "0") << "the size took its place";
    EXPECT_EQ(s.getADCLimits().size, 2000000000u);
    ASSERT_TRUE(s.setValue("split_time_sd", "300"));
    ASSERT_TRUE(s.setValue("split_size_sd", "1073741824"));
    EXPECT_EQ(s.getValue("split_time_sd"), "0");
    EXPECT_EQ(s.getADCLimits().splitSize, 1073741824u);
    // a 0 clears only its own
    ASSERT_TRUE(s.setValue("limit_time_sd", "0"));
    EXPECT_EQ(s.getADCLimits().size, 2000000000u);
    // the GPIO input has its own
    EXPECT_EQ(s.getGPIOInLimits().size, 0u);
    ASSERT_TRUE(s.setValue("gpio_in_split_size_sd", "1000"));
    EXPECT_EQ(s.getGPIOInLimits().splitSize, 1000u);
    EXPECT_EQ(s.getADCLimits().splitSize, 1073741824u);
}

TEST(StreamSettingsKeys, ABadTimeOfALimitIsRejected) {
    CStreamSettings s;
    const std::string before = s.toJson();
    EXPECT_FALSE(s.setValue("limit_time_sd", "-1"));
    EXPECT_FALSE(s.setValue("split_time_sd", "1e12")) << "more than a year";
    EXPECT_FALSE(s.setValue("split_time_sd", "nan"));
    EXPECT_FALSE(s.setValue("limit_size_sd", "-5"));
    EXPECT_FALSE(s.setValue("gpio_in_limit_time_sd", "x"));
    EXPECT_EQ(s.getValue("gpio_in_no_such_sd"), "ERROR");
    EXPECT_EQ(s.toJson(), before);
    CStreamSettings::FileLimits bad;
    bad.time = -3;
    EXPECT_FALSE(s.setADCLimits(bad));
}

TEST(StreamSettingsKeys, TheLimitsGoThroughTheJson) {
    CStreamSettings a;
    CStreamSettings::FileLimits l;
    l.time = 12.5;
    l.splitSize = 4096;
    ASSERT_TRUE(a.setADCLimits(l));
    CStreamSettings::FileLimits g;
    g.size = 777;
    g.splitTime = 3;
    ASSERT_TRUE(a.setGPIOInLimits(g));
    CStreamSettings b;
    ASSERT_TRUE(b.parseJson(a.toJson()));
    EXPECT_EQ(b.getADCLimits().time, 12.5);
    EXPECT_EQ(b.getADCLimits().splitSize, 4096u);
    EXPECT_EQ(b.getGPIOInLimits().size, 777u);
    EXPECT_EQ(b.getGPIOInLimits().splitTime, 3.0);
    // a configuration of before has none of them: no limit, one file
    Json::Value old;
    std::istringstream in(a.toJson());
    ASSERT_TRUE(Json::parseFromStream(Json::CharReaderBuilder(), in, &old, nullptr));
    for (const char* k : {"limit_time_sd", "limit_size_sd", "split_size_sd", "split_time_sd"}) {
        old["adc_streaming"].removeMember(k);
        old["gpio_streaming"].removeMember(std::string("gpio_in_") + k);
    }
    CStreamSettings c;
    ASSERT_TRUE(c.parseJson(Json::writeString(Json::StreamWriterBuilder(), old)));
    EXPECT_EQ(c.getADCLimits().time, 0.0);
    EXPECT_EQ(c.getADCLimits().splitSize, 0u);
    EXPECT_EQ(c.getGPIOInLimits().size, 0u);
}

// The JSON keeps the rule of the keys: one limit and one split at a time
TEST(StreamSettingsKeys, TheJsonKeepsOneLimitAndOneSplit) {
    CStreamSettings a;
    Json::Value j;
    {
        std::istringstream in(a.toJson());
        ASSERT_TRUE(Json::parseFromStream(Json::CharReaderBuilder(), in, &j, nullptr));
    }
    // both given: the time, the size of a part
    j["adc_streaming"]["limit_time_sd"] = 60.0;
    j["adc_streaming"]["limit_size_sd"] = Json::UInt64(1000000);
    j["adc_streaming"]["split_size_sd"] = Json::UInt64(2000000);
    j["adc_streaming"]["split_time_sd"] = 10.0;
    CStreamSettings b;
    ASSERT_TRUE(b.parseJson(Json::writeString(Json::StreamWriterBuilder(), j)));
    EXPECT_EQ(b.getADCLimits().time, 60.0);
    EXPECT_EQ(b.getADCLimits().size, 0u);
    EXPECT_EQ(b.getADCLimits().splitSize, 2000000u);
    EXPECT_EQ(b.getADCLimits().splitTime, 0.0);
    // the struct alike
    CStreamSettings::FileLimits both;
    both.time = 5;
    both.size = 10;
    both.splitSize = 20;
    both.splitTime = 30;
    ASSERT_TRUE(b.setGPIOInLimits(both));
    EXPECT_EQ(b.getGPIOInLimits().size, 0u);
    EXPECT_EQ(b.getGPIOInLimits().splitTime, 0.0);
}
