#include "value.h"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <stdexcept>

using namespace TDMS;

namespace {

constexpr std::int64_t kSecondsFrom1904To1970 = 2082844800LL;
constexpr std::uint64_t kNsPerSecond = 1000000000ULL;

auto isFloatType(TDMSType type) -> bool {
    return type == TDMSType::SingleFloat || type == TDMSType::DoubleFloat || type == TDMSType::SingleFloatWithUnit || type == TDMSType::DoubleFloatWithUnit;
}

auto isSignedType(TDMSType type) -> bool {
    return type == TDMSType::Integer8 || type == TDMSType::Integer16 || type == TDMSType::Integer32 || type == TDMSType::Integer64;
}

auto isUnsignedType(TDMSType type) -> bool {
    return type == TDMSType::UnsignedInteger8 || type == TDMSType::UnsignedInteger16 || type == TDMSType::UnsignedInteger32 || type == TDMSType::UnsignedInteger64;
}

template <typename T>
auto load(const std::vector<std::uint8_t>& bytes) -> T {
    T v{};
    std::memcpy(&v, bytes.data(), sizeof(T));
    return v;
}

}  // namespace

// ns * 2^64 / 1e9 without 128 bit arithmetic: 2^64 / 1e9 = 18446744073.709551616
auto TimeStamp::fromUnix(std::int64_t seconds, std::uint32_t nanoseconds) -> TimeStamp {
    TimeStamp t;
    seconds += nanoseconds / kNsPerSecond;
    const std::uint64_t ns = nanoseconds % kNsPerSecond;
    t.seconds = seconds + kSecondsFrom1904To1970;
    t.fraction = ns * 18446744073ULL + (ns * 709551616ULL + kNsPerSecond / 2) / kNsPerSecond;
    return t;
}

auto TimeStamp::fromUnixNs(std::int64_t nanoseconds) -> TimeStamp {
    std::int64_t seconds = nanoseconds / static_cast<std::int64_t>(kNsPerSecond);
    std::int64_t rest = nanoseconds % static_cast<std::int64_t>(kNsPerSecond);
    if (rest < 0) {
        rest += kNsPerSecond;
        seconds--;
    }
    return fromUnix(seconds, static_cast<std::uint32_t>(rest));
}

auto TimeStamp::unixSeconds() const -> std::int64_t {
    return seconds - kSecondsFrom1904To1970;
}

auto TimeStamp::unixNs() const -> std::int64_t {
    const std::uint64_t hi = fraction >> 32;
    const std::uint64_t lo = fraction & 0xFFFFFFFFULL;
    const std::uint64_t ns = (hi * kNsPerSecond + ((lo * kNsPerSecond) >> 32) + (1ULL << 31)) >> 32;
    return unixSeconds() * static_cast<std::int64_t>(kNsPerSecond) + static_cast<std::int64_t>(ns);
}

Value::Value(bool value) {
    const std::uint8_t b = value ? 1 : 0;
    set(TDMSType::Boolean, &b, 1);
}

Value::Value(const char* value) : Value(std::string(value ? value : "")) {}

Value::Value(const std::string& value) {
    set(TDMSType::String, value.data(), value.size());
}

Value::Value(const TimeStamp& value) {
    std::uint8_t payload[16];
    std::memcpy(payload, &value.fraction, 8);
    std::memcpy(payload + 8, &value.seconds, 8);
    set(TDMSType::TimeStamp, payload, sizeof(payload));
}

auto Value::fromBytes(TDMSType type, const void* data, std::size_t size) -> Value {
    if (type != TDMSType::String && size != DataType::GetLength(type))
        throw std::invalid_argument("[ERROR] Value::fromBytes: the payload does not match the width of the type");
    Value v;
    v.set(type, data, size);
    return v;
}

auto Value::set(TDMSType type, const void* data, std::size_t size) -> void {
    m_type = type;
    const auto* p = static_cast<const std::uint8_t*>(data);
    if (p == nullptr || size == 0)
        m_bytes.clear();
    else
        m_bytes.assign(p, p + size);
}

auto Value::isNumber() const -> bool {
    return isFloatType(m_type) || isSignedType(m_type) || isUnsignedType(m_type);
}

auto Value::asDouble() const -> std::optional<double> {
    if (m_bytes.size() != DataType::GetLength(m_type))
        return std::nullopt;
    switch (m_type) {
        case TDMSType::Boolean:
            return m_bytes[0] ? 1.0 : 0.0;
        case TDMSType::SingleFloat:
        case TDMSType::SingleFloatWithUnit:
            return static_cast<double>(load<float>(m_bytes));
        case TDMSType::DoubleFloat:
        case TDMSType::DoubleFloatWithUnit:
            return load<double>(m_bytes);
        default:
            break;
    }
    if (isSignedType(m_type))
        return static_cast<double>(*asInt64());
    if (isUnsignedType(m_type))
        return static_cast<double>(*asUInt64());
    return std::nullopt;
}

auto Value::asInt64() const -> std::optional<std::int64_t> {
    if (m_bytes.size() != DataType::GetLength(m_type))
        return std::nullopt;
    switch (m_type) {
        case TDMSType::Boolean:
            return m_bytes[0] ? 1 : 0;
        case TDMSType::Integer8:
            return load<std::int8_t>(m_bytes);
        case TDMSType::Integer16:
            return load<std::int16_t>(m_bytes);
        case TDMSType::Integer32:
            return load<std::int32_t>(m_bytes);
        case TDMSType::Integer64:
            return load<std::int64_t>(m_bytes);
        default:
            break;
    }
    if (isUnsignedType(m_type)) {
        auto u = asUInt64();
        if (*u > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
            return std::nullopt;
        return static_cast<std::int64_t>(*u);
    }
    if (isFloatType(m_type)) {
        const double d = *asDouble();
        // 2^63 is exact in double; the range is [-2^63, 2^63)
        if (!std::isfinite(d) || std::trunc(d) != d || d < -9223372036854775808.0 || d >= 9223372036854775808.0)
            return std::nullopt;
        return static_cast<std::int64_t>(d);
    }
    return std::nullopt;
}

auto Value::asUInt64() const -> std::optional<std::uint64_t> {
    if (m_bytes.size() != DataType::GetLength(m_type))
        return std::nullopt;
    switch (m_type) {
        case TDMSType::Boolean:
            return m_bytes[0] ? 1u : 0u;
        case TDMSType::UnsignedInteger8:
            return load<std::uint8_t>(m_bytes);
        case TDMSType::UnsignedInteger16:
            return load<std::uint16_t>(m_bytes);
        case TDMSType::UnsignedInteger32:
            return load<std::uint32_t>(m_bytes);
        case TDMSType::UnsignedInteger64:
            return load<std::uint64_t>(m_bytes);
        default:
            break;
    }
    if (isSignedType(m_type)) {
        auto s = asInt64();
        if (*s < 0)
            return std::nullopt;
        return static_cast<std::uint64_t>(*s);
    }
    if (isFloatType(m_type)) {
        const double d = *asDouble();
        if (!std::isfinite(d) || std::trunc(d) != d || d < 0 || d >= 18446744073709551616.0)
            return std::nullopt;
        return static_cast<std::uint64_t>(d);
    }
    return std::nullopt;
}

auto Value::toString() const -> std::string {
    char buf[64];
    switch (m_type) {
        case TDMSType::Empty:
            return "";
        case TDMSType::String:
            return std::string(m_bytes.begin(), m_bytes.end());
        case TDMSType::Boolean:
            return m_bytes.size() == 1 && m_bytes[0] ? "true" : "false";
        case TDMSType::TimeStamp: {
            auto t = get<TimeStamp>();
            if (!t)
                return "";
            const std::int64_t ns = t->unixNs();
            std::int64_t sec = ns / static_cast<std::int64_t>(kNsPerSecond);
            std::int64_t frac = ns % static_cast<std::int64_t>(kNsPerSecond);
            if (frac < 0) {
                frac += kNsPerSecond;
                sec--;
            }
            std::time_t tt = static_cast<std::time_t>(sec);
            std::tm tm{};
#ifdef _WIN32
            gmtime_s(&tm, &tt);
#else
            gmtime_r(&tt, &tm);
#endif
            std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%09lldZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                          static_cast<long long>(frac));
            return buf;
        }
        default:
            break;
    }
    if (isFloatType(m_type)) {
        auto d = asDouble();
        if (!d)
            return "";
        std::snprintf(buf, sizeof(buf), "%.17g", *d);
        return buf;
    }
    if (isSignedType(m_type)) {
        auto v = asInt64();
        return v ? std::to_string(*v) : "";
    }
    if (isUnsignedType(m_type)) {
        auto v = asUInt64();
        return v ? std::to_string(*v) : "";
    }
    return "";
}

auto Value::toDataType() const -> DataType {
    DataType d;
    d.InitBytes(m_type, m_bytes.data(), m_bytes.size());
    return d;
}

auto Value::fromDataType(const DataType& value) -> Value {
    Value v;
    if (value.GetDataType() == TDMSType::String) {
        const std::string s = value.GetDataString();
        v.set(TDMSType::String, s.data(), s.size());
    } else {
        v.set(value.GetDataType(), value.GetRawData(), value.GetRawData() ? value.GetLength() : 0);
    }
    return v;
}
