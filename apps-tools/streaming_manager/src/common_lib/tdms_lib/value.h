#ifndef TDMS_LIB_VALUE_H
#define TDMS_LIB_VALUE_H

#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "data_type.h"

namespace TDMS {

// A TDMS timestamp: seconds since 1904-01-01 00:00 UTC and a fraction of a second in 2^-64 s.
struct TimeStamp {
    std::int64_t seconds = 0;
    std::uint64_t fraction = 0;

    static auto fromUnix(std::int64_t seconds, std::uint32_t nanoseconds = 0) -> TimeStamp;
    static auto fromUnixNs(std::int64_t nanoseconds) -> TimeStamp;
    auto unixSeconds() const -> std::int64_t;
    auto unixNs() const -> std::int64_t;  // rounded to the nearest nanosecond

    auto operator==(const TimeStamp& other) const -> bool { return seconds == other.seconds && fraction == other.fraction; }
    auto operator!=(const TimeStamp& other) const -> bool { return !(*this == other); }
};

// The TDMS type of a C++ number: int16_t -> Integer16, double -> DoubleFloat, and so on
template <typename T>
constexpr auto typeOf() -> TDMSType {
    static_assert(std::is_arithmetic_v<T> && !std::is_same_v<T, char>, "a number type; char has no fixed signedness, use int8_t or uint8_t");
    if constexpr (std::is_same_v<T, bool>) {
        return TDMSType::Boolean;
    } else if constexpr (std::is_floating_point_v<T>) {
        static_assert(sizeof(T) == 4 || sizeof(T) == 8, "only float and double have a TDMS type");
        return sizeof(T) == 4 ? TDMSType::SingleFloat : TDMSType::DoubleFloat;
    } else if constexpr (std::is_signed_v<T>) {
        return sizeof(T) == 1 ? TDMSType::Integer8 : sizeof(T) == 2 ? TDMSType::Integer16 : sizeof(T) == 4 ? TDMSType::Integer32 : TDMSType::Integer64;
    } else {
        return sizeof(T) == 1   ? TDMSType::UnsignedInteger8
               : sizeof(T) == 2 ? TDMSType::UnsignedInteger16
               : sizeof(T) == 4 ? TDMSType::UnsignedInteger32
                                : TDMSType::UnsignedInteger64;
    }
}

// A property value: a number of any TDMS width, a bool, a string or a timestamp.
// The C++ type picks the TDMS type: int16_t -> Integer16, double -> DoubleFloat, and so on.
class Value {
   public:
    Value() = default;
    Value(bool value);
    Value(const char* value);
    Value(const std::string& value);
    Value(const TimeStamp& value);
    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
    Value(T value) {
        set(typeOf<T>(), &value, sizeof(T));
    }

    // A value of an explicit type from its little-endian payload (GetLength(type) bytes, any length for String)
    static auto fromBytes(TDMSType type, const void* data, std::size_t size) -> Value;

    auto type() const -> TDMSType { return m_type; }
    auto empty() const -> bool { return m_type == TDMSType::Empty; }
    auto isNumber() const -> bool;
    auto bytes() const -> const std::vector<std::uint8_t>& { return m_bytes; }

    // The value as T: a number or a bool converts to any arithmetic T it fits in (an integer T takes
    // only whole numbers in its range), a string only to std::string, a timestamp only to TimeStamp.
    // Anything else gives nothing.
    template <typename T>
    auto get() const -> std::optional<T>;

    // For a reader of strings that does not care about the type
    auto toString() const -> std::string;

    auto operator==(const Value& other) const -> bool { return m_type == other.m_type && m_bytes == other.m_bytes; }
    auto operator!=(const Value& other) const -> bool { return !(*this == other); }

    // Bridges to the property type of the File/Reader/Writer API
    auto toDataType() const -> DataType;
    static auto fromDataType(const DataType& value) -> Value;

   private:
    auto set(TDMSType type, const void* data, std::size_t size) -> void;
    auto asDouble() const -> std::optional<double>;
    auto asInt64() const -> std::optional<std::int64_t>;
    auto asUInt64() const -> std::optional<std::uint64_t>;

    TDMSType m_type = TDMSType::Empty;
    std::vector<std::uint8_t> m_bytes;
};

using Properties = std::map<std::string, Value>;

template <typename T>
auto Value::get() const -> std::optional<T> {
    if constexpr (std::is_same_v<T, std::string>) {
        if (m_type != TDMSType::String)
            return std::nullopt;
        return std::string(m_bytes.begin(), m_bytes.end());
    } else if constexpr (std::is_same_v<T, TimeStamp>) {
        if (m_type != TDMSType::TimeStamp || m_bytes.size() != 16)
            return std::nullopt;
        TimeStamp t;
        std::memcpy(&t.fraction, m_bytes.data(), 8);
        std::memcpy(&t.seconds, m_bytes.data() + 8, 8);
        return t;
    } else {
        static_assert(std::is_arithmetic_v<T>, "get<T>: a number, bool, std::string or TimeStamp");
        if constexpr (std::is_floating_point_v<T>) {
            auto v = asDouble();
            return v ? std::optional<T>(static_cast<T>(*v)) : std::nullopt;
        } else if constexpr (std::is_same_v<T, bool>) {
            auto v = asDouble();
            return v ? std::optional<T>(*v != 0) : std::nullopt;
        } else if constexpr (std::is_signed_v<T>) {
            auto v = asInt64();
            if (!v || *v < std::numeric_limits<T>::min() || *v > std::numeric_limits<T>::max())
                return std::nullopt;
            return static_cast<T>(*v);
        } else {
            auto v = asUInt64();
            if (!v || *v > std::numeric_limits<T>::max())
                return std::nullopt;
            return static_cast<T>(*v);
        }
    }
}

// The value of `key`, if it is there and converts to T
template <typename T>
auto getProperty(const Properties& properties, const std::string& key) -> std::optional<T> {
    auto it = properties.find(key);
    if (it == properties.end())
        return std::nullopt;
    return it->second.template get<T>();
}

}  // namespace TDMS

#endif
