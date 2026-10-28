/**
 * TDMS::Value and TimeStamp - property values of the TDMS API.
 *
 * The C++ type of a value picks its TDMS type, get<T>() converts only when the value fits, and a
 * timestamp keeps nanoseconds through the 2^-64 s fraction of the file format.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>

#include "spec_builder.h"
#include "tdms.h"

using TDMS::TDMSType;
using TDMS::TimeStamp;
using TDMS::Value;

TEST(Value, TheCppTypePicksTheTdmsType) {
    EXPECT_EQ(Value(int8_t(1)).type(), TDMSType::Integer8);
    EXPECT_EQ(Value(int16_t(1)).type(), TDMSType::Integer16);
    EXPECT_EQ(Value(int32_t(1)).type(), TDMSType::Integer32);
    EXPECT_EQ(Value(int64_t(1)).type(), TDMSType::Integer64);
    EXPECT_EQ(Value(uint8_t(1)).type(), TDMSType::UnsignedInteger8);
    EXPECT_EQ(Value(uint16_t(1)).type(), TDMSType::UnsignedInteger16);
    EXPECT_EQ(Value(uint32_t(1)).type(), TDMSType::UnsignedInteger32);
    EXPECT_EQ(Value(uint64_t(1)).type(), TDMSType::UnsignedInteger64);
    EXPECT_EQ(Value(1.0f).type(), TDMSType::SingleFloat);
    EXPECT_EQ(Value(1.0).type(), TDMSType::DoubleFloat);
    EXPECT_EQ(Value(true).type(), TDMSType::Boolean);
    EXPECT_EQ(Value("text").type(), TDMSType::String);
    EXPECT_EQ(Value(std::string("text")).type(), TDMSType::String);
    EXPECT_EQ(Value(TimeStamp{}).type(), TDMSType::TimeStamp);
    EXPECT_TRUE(Value().empty());
    // long and long long are both 64 bit on the targets; either is Integer64
    EXPECT_EQ(Value(1LL).type(), TDMSType::Integer64);
    EXPECT_EQ(Value(1ULL).type(), TDMSType::UnsignedInteger64);
}

TEST(Value, ThePayloadIsTheLittleEndianValue) {
    EXPECT_EQ(Value(int16_t(0x1234)).bytes(), (std::vector<std::uint8_t>{0x34, 0x12}));
    EXPECT_EQ(Value(true).bytes(), (std::vector<std::uint8_t>{1}));
    EXPECT_EQ(Value("ab").bytes(), (std::vector<std::uint8_t>{'a', 'b'}));
    EXPECT_EQ(Value(1.5).bytes().size(), 8u);
}

TEST(Value, NumbersConvertWhenTheyFit) {
    Value v(int32_t(300));
    EXPECT_EQ(v.get<int32_t>(), 300);
    EXPECT_EQ(v.get<int64_t>(), 300);
    EXPECT_EQ(v.get<uint16_t>(), 300u);
    EXPECT_EQ(v.get<double>(), 300.0);
    EXPECT_EQ(v.get<bool>(), true);
    EXPECT_FALSE(v.get<int8_t>()) << "300 does not fit";
    EXPECT_FALSE(v.get<uint8_t>());
    EXPECT_FALSE(Value(int8_t(-1)).get<uint32_t>()) << "negative into unsigned";
    EXPECT_EQ(Value(uint64_t(std::numeric_limits<uint64_t>::max())).get<uint64_t>(), std::numeric_limits<uint64_t>::max());
    EXPECT_FALSE(Value(uint64_t(std::numeric_limits<uint64_t>::max())).get<int64_t>());
}

TEST(Value, AFloatGoesToAnIntegerOnlyWhenWhole) {
    EXPECT_EQ(Value(8.0).get<uint32_t>(), 8u);
    EXPECT_FALSE(Value(8.5).get<uint32_t>());
    EXPECT_FALSE(Value(-1.0).get<uint32_t>());
    EXPECT_EQ(Value(-1.0f).get<int16_t>(), -1);
    EXPECT_FALSE(Value(std::numeric_limits<double>::infinity()).get<int64_t>());
    EXPECT_FALSE(Value(9223372036854775808.0).get<int64_t>()) << "2^63 is out of range";
    EXPECT_DOUBLE_EQ(*Value(0.25f).get<double>(), 0.25);
}

TEST(Value, StringsAndTimestampsDoNotPassForNumbers) {
    EXPECT_FALSE(Value("8").get<int32_t>());
    EXPECT_FALSE(Value(int32_t(8)).get<std::string>());
    EXPECT_FALSE(Value(TimeStamp{}).get<int64_t>());
    EXPECT_FALSE(Value(int64_t(0)).get<TimeStamp>());
    EXPECT_EQ(Value("8").get<std::string>(), std::string("8"));
    EXPECT_FALSE(Value().get<int32_t>());
}

TEST(Value, ToStringIsReadable) {
    EXPECT_EQ(Value(int16_t(-5)).toString(), "-5");
    EXPECT_EQ(Value(uint64_t(18446744073709551615ULL)).toString(), "18446744073709551615");
    EXPECT_EQ(Value(0.5).toString(), "0.5");
    EXPECT_EQ(Value(true).toString(), "true");
    EXPECT_EQ(Value("ok").toString(), "ok");
    EXPECT_EQ(Value(TimeStamp::fromUnix(1700000000, 123456789)).toString(), "2023-11-14T22:13:20.123456789Z");
}

TEST(Value, FromBytesChecksTheWidth) {
    const std::uint8_t two[2] = {1, 0};
    EXPECT_EQ(Value::fromBytes(TDMSType::Integer16, two, 2), Value(int16_t(1)));
    EXPECT_THROW(Value::fromBytes(TDMSType::Integer32, two, 2), std::invalid_argument);
    EXPECT_EQ(Value::fromBytes(TDMSType::String, "abc", 3), Value("abc"));
}

TEST(Value, TheOldPropertyTypeConvertsBothWays) {
    for (const Value& v : {Value(int16_t(-3)), Value(2.5), Value("text"), Value(true), Value(TimeStamp::fromUnix(10, 5))}) {
        EXPECT_EQ(Value::fromDataType(v.toDataType()), v) << v.toString();
    }
}

TEST(TimeStamp, TheEpochIs1904) {
    EXPECT_EQ(TimeStamp::fromUnix(0).seconds, 2082844800);
    EXPECT_EQ(TimeStamp::fromUnix(0).fraction, 0u);
    EXPECT_EQ(TimeStamp::fromUnix(1, 0).unixSeconds(), 1);
}

TEST(TimeStamp, NanosecondsSurviveTheFraction) {
    for (std::uint32_t ns : {0u, 1u, 2u, 500000000u, 999999999u, 123456789u}) {
        EXPECT_EQ(TimeStamp::fromUnix(1700000000, ns).unixNs(), 1700000000LL * 1000000000LL + ns) << ns;
    }
    // half a second is exactly 2^63
    EXPECT_EQ(TimeStamp::fromUnix(0, 500000000).fraction, 1ULL << 63);
}

TEST(TimeStamp, NegativeNanosecondsCountBackFromTheUnixEpoch) {
    const auto t = TimeStamp::fromUnixNs(-1);
    EXPECT_EQ(t.unixSeconds(), -1);
    EXPECT_EQ(t.unixNs(), -1);
    EXPECT_EQ(TimeStamp::fromUnixNs(1234567890123456789LL).unixNs(), 1234567890123456789LL);
}

TEST(Properties, GetPropertyFindsAndConverts) {
    TDMS::Properties p{{"rate", Value(uint32_t(125000000))}, {"name", Value("ch1")}};
    EXPECT_EQ(TDMS::getProperty<double>(p, "rate"), 125e6);
    EXPECT_EQ(TDMS::getProperty<std::string>(p, "name"), std::string("ch1"));
    EXPECT_FALSE(TDMS::getProperty<int32_t>(p, "missing"));
    EXPECT_FALSE(TDMS::getProperty<int32_t>(p, "name"));
}
