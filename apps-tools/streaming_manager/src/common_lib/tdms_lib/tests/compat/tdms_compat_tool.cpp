// The C++ side of the npTDMS compatibility test (compat_nptdms.py):
//   tdms_compat_tool write <file>  writes the reference file with SegmentBuilder
//   tdms_compat_tool dump <file>   prints what FileReader reads from a file, as JSON
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "tdms.h"

using namespace TDMS;

namespace {

// A number for Python's json module, which takes NaN and Infinity
auto number(const Value& v) -> std::string {
    if (auto d = v.get<double>(); d && (v.type() == TDMSType::SingleFloat || v.type() == TDMSType::DoubleFloat)) {
        if (std::isnan(*d))
            return "NaN";
        if (std::isinf(*d))
            return *d > 0 ? "Infinity" : "-Infinity";
    }
    return v.toString();
}

auto json(const std::string& text) -> std::string {
    std::string out = "\"";
    for (unsigned char c : text) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += static_cast<char>(c);
        } else if (c < 0x20) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\u%04x", c);
            out += buf;
        } else {
            out += static_cast<char>(c);
        }
    }
    return out + "\"";
}

auto json(const Value& v) -> std::string {
    std::string value;
    switch (v.type()) {
        case TDMSType::String:
            value = json(*v.get<std::string>());
            break;
        case TDMSType::Boolean:
            value = *v.get<bool>() ? "true" : "false";
            break;
        case TDMSType::TimeStamp:
            value = std::to_string(v.get<TimeStamp>()->unixNs());
            break;
        default:
            value = number(v);
            break;
    }
    return "{\"type\": " + std::to_string(static_cast<unsigned>(v.type())) + ", \"value\": " + value + "}";
}

auto json(const Properties& p) -> std::string {
    std::string out = "{";
    for (const auto& [k, v] : p)
        out += (out.size() > 1 ? ", " : "") + json(k) + ": " + json(v);
    return out + "}";
}

template <typename T>
auto samples(FileReader& f, const ChannelInfo& c) -> std::string {
    std::string out = "[";
    for (const auto& v : f.readAll<T>(c.group, c.name)) {
        if (out.size() > 1)
            out += ", ";
        out += number(Value(v));
    }
    return out + "]";
}

auto samples(FileReader& f, const ChannelInfo& c) -> std::string {
    switch (c.type) {
        case TDMSType::Integer8:
            return samples<int8_t>(f, c);
        case TDMSType::Integer16:
            return samples<int16_t>(f, c);
        case TDMSType::Integer32:
            return samples<int32_t>(f, c);
        case TDMSType::Integer64:
            return samples<int64_t>(f, c);
        case TDMSType::UnsignedInteger8:
            return samples<uint8_t>(f, c);
        case TDMSType::UnsignedInteger16:
            return samples<uint16_t>(f, c);
        case TDMSType::UnsignedInteger32:
            return samples<uint32_t>(f, c);
        case TDMSType::UnsignedInteger64:
            return samples<uint64_t>(f, c);
        case TDMSType::SingleFloat:
            return samples<float>(f, c);
        case TDMSType::DoubleFloat:
            return samples<double>(f, c);
        default:
            return "null";
    }
}

auto dump(const std::string& path) -> int {
    FileReader f;
    if (!f.open(path)) {
        std::cerr << f.error() << "\n";
        return 1;
    }
    std::cout << "{\"properties\": " << json(f.properties()) << ",\n \"groups\": {";
    bool first = true;
    for (const auto& g : f.groups()) {
        std::cout << (first ? "" : ", ") << json(g) << ": " << json(f.groupProperties(g));
        first = false;
    }
    std::cout << "},\n \"channels\": [";
    first = true;
    for (const auto& c : f.channels()) {
        std::cout << (first ? "\n  " : ",\n  ") << "{\"group\": " << json(c.group) << ", \"name\": " << json(c.name)
                  << ", \"type\": " << static_cast<unsigned>(c.type) << ", \"count\": " << c.count << ", \"properties\": " << json(c.properties)
                  << ", \"data\": " << samples(f, c) << "}";
        first = false;
    }
    std::cout << "]}\n";
    return 0;
}

// Keep in step with REFERENCE in compat_nptdms.py
auto write(const std::string& path) -> int {
    FileWriter out;
    if (!out.open(path))
        return 1;
    for (int seg = 0; seg < 3; seg++) {
        SegmentBuilder s;
        if (seg == 0) {
            s.setProperty("name", "reference");
            s.setProperty("i8", int8_t(-8));
            s.setProperty("i16", int16_t(-16));
            s.setProperty("i32", int32_t(-32));
            s.setProperty("i64", int64_t(-64));
            s.setProperty("u8", uint8_t(8));
            s.setProperty("u16", uint16_t(16));
            s.setProperty("u32", uint32_t(32));
            s.setProperty("u64", uint64_t(18446744073709551615ULL));
            s.setProperty("f32", 0.5f);
            s.setProperty("f64", -2.25);
            s.setProperty("flag", true);
            s.setProperty("start", TimeStamp::fromUnix(1700000000, 123456789));
            s.setProperty("text", "line\nwith \"quotes\" and π");
            s.setGroupProperty("Group", "decimation", uint32_t(8));
            setWaveform(s, "Group", "ch1", Waveform{TimeStamp::fromUnix(1700000000, 0), 64e-9, 0.5});
            s.setChannelProperty("Group", "ch1", "unit_string", "V");
            s.setChannelProperty("It's", "a'b", "quoted", true);
        }
        s.setGroupProperty("Group", "segment", int32_t(seg));
        std::vector<int16_t> ch1;
        std::vector<float> ch2;
        for (int i = 0; i < 4; i++) {
            ch1.push_back(static_cast<int16_t>(seg * 100 + i - 2));
            ch2.push_back(static_cast<float>(seg) + i * 0.25f);
        }
        s.copySamples("Group", "ch1", ch1);
        if (seg != 1)
            s.copySamples("Group", "ch2", ch2);
        const std::vector<uint64_t> big{uint64_t(seg), 18446744073709551615ULL};
        s.copySamples("It's", "a'b", big);
        if (!out.write(s))
            return 1;
    }
    return out.close() ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "write")
        return write(argv[2]);
    if (argc == 3 && std::string(argv[1]) == "dump")
        return dump(argv[2]);
    std::cerr << "usage: tdms_compat_tool write|dump <file>\n";
    return 2;
}
