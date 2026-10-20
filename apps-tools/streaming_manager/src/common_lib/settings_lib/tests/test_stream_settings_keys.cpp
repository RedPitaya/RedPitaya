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

#include <string>
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
    {"samples_limit_sd", "123456789012"},
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
    {"file_sd", "/tmp/generated.wav"},
    {"file_type_sd", "TDMS"},
    {"dac_pass_mode", "DAC_FILE"},
    {"repeat", "DAC_REP_INF"},
    {"repeatCount", "9"},
    {"channel_gain_1", "X5"},
    {"channel_gain_2", "X5"},
    {"block_size", "131072"},
    {"adc_size", "1000"},
    {"dac_size", "2000"},
    {"gpio_size", "3000"},
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
    {"gpio_in_streaming", "ON"},
    {"gpio_out_streaming", "ON"},
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

    // file_type_sd goes through setDACFileType, which refuses BIN, and setValue
    // now passes that answer on instead of reporting success.
    EXPECT_TRUE(s.setValue("file_type_sd", "TDMS"));
    EXPECT_FALSE(s.setValue("file_type_sd", "BIN"));
    EXPECT_EQ(s.getValue("file_type_sd"), "TDMS") << "and nothing was written";
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

TEST(StreamSettingsKeys, TheFilePathKeyTakesAnyString) {
    CStreamSettings s;
    EXPECT_TRUE(s.setValue("file_sd", ""));
    EXPECT_EQ(s.getValue("file_sd"), "");
    EXPECT_TRUE(s.setValue("file_sd", "  a b/c.wav  "));
    EXPECT_EQ(s.getValue("file_sd"), "  a b/c.wav  ") << "no trimming, no validation";
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
