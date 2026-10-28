#include "segment_builder.h"

#include <cstring>
#include <sstream>
#include <stdexcept>

#include "path.h"

using namespace TDMS;

namespace {

constexpr std::uint32_t kTocMetaData = 1 << 1;
constexpr std::uint32_t kTocNewObjList = 1 << 2;
constexpr std::uint32_t kTocRawData = 1 << 3;
constexpr std::int32_t kVersion = 4713;
constexpr std::uint32_t kNoRawData = 0xFFFFFFFF;
constexpr std::uint32_t kRawIndexLength = 20;

template <typename T>
auto put(std::string& out, T value) -> void {
    char raw[sizeof(T)];
    std::memcpy(raw, &value, sizeof(T));
    out.append(raw, sizeof(T));
}

auto putString(std::string& out, const std::string& text) -> void {
    put<std::uint32_t>(out, static_cast<std::uint32_t>(text.size()));
    out += text;
}

auto putProperties(std::string& out, const Properties& properties) -> void {
    put<std::uint32_t>(out, static_cast<std::uint32_t>(properties.size()));
    for (const auto& [key, value] : properties) {
        putString(out, key);
        put<std::uint32_t>(out, static_cast<std::uint32_t>(value.type()));
        if (value.type() == TDMSType::String)
            put<std::uint32_t>(out, static_cast<std::uint32_t>(value.bytes().size()));
        out.append(reinterpret_cast<const char*>(value.bytes().data()), value.bytes().size());
    }
}

}  // namespace

auto SegmentBuilder::group(const std::string& name) -> Group& {
    for (auto& g : m_groups)
        if (g.name == name)
            return g;
    m_groups.push_back(Group{name, {}, {}});
    return m_groups.back();
}

auto SegmentBuilder::channel(const std::string& groupName, const std::string& name) -> Channel& {
    auto& g = group(groupName);
    for (auto& c : g.channels)
        if (c.name == name)
            return c;
    g.channels.push_back(Channel{name, {}, TDMSType::Empty, 0, {}});
    return g.channels.back();
}

auto SegmentBuilder::setProperty(const std::string& key, const Value& value) -> void {
    m_properties[key] = value;
}

auto SegmentBuilder::setGroupProperty(const std::string& groupName, const std::string& key, const Value& value) -> void {
    group(groupName).properties[key] = value;
}

auto SegmentBuilder::setChannelProperty(const std::string& groupName, const std::string& channelName, const std::string& key, const Value& value) -> void {
    channel(groupName, channelName).properties[key] = value;
}

auto SegmentBuilder::copySamples(const std::string& groupName, const std::string& channelName, TDMSType type, const void* data, std::uint64_t count) -> void {
    const std::uint64_t size = count * DataType::GetLength(type);
    std::shared_ptr<std::uint8_t[]> copy(new std::uint8_t[size ? size : 1]);
    if (size)
        std::memcpy(copy.get(), data, size);
    addSamples(groupName, channelName, type, std::shared_ptr<const std::uint8_t[]>(copy), count);
}

auto SegmentBuilder::addSamples(const std::string& groupName, const std::string& channelName, TDMSType type, std::shared_ptr<const std::uint8_t[]> data,
                                std::uint64_t count) -> void {
    const std::uint32_t width = type == TDMSType::String ? 0 : DataType::GetLength(type);
    if (width == 0 || type == TDMSType::Void)
        throw std::invalid_argument("[ERROR] SegmentBuilder: channel samples need a fixed width type");
    auto& c = channel(groupName, channelName);
    if (c.type != TDMSType::Empty && c.type != type)
        throw std::invalid_argument("[ERROR] SegmentBuilder: the channel already has samples of another type in this segment");
    c.type = type;
    if (count == 0)
        return;
    c.count += count;
    c.blocks.push_back(Block{std::move(data), count * width});
}

auto SegmentBuilder::empty() const -> bool {
    return m_properties.empty() && m_groups.empty();
}

auto SegmentBuilder::clear() -> void {
    m_properties.clear();
    m_groups.clear();
}

auto SegmentBuilder::head() const -> std::string {
    std::string meta;
    std::uint64_t rawSize = 0;
    std::uint32_t objects = 1;
    for (const auto& g : m_groups)
        objects += 1 + static_cast<std::uint32_t>(g.channels.size());
    put<std::uint32_t>(meta, objects);

    putString(meta, "/");
    put<std::uint32_t>(meta, kNoRawData);
    putProperties(meta, m_properties);
    for (const auto& g : m_groups) {
        putString(meta, groupPath(g.name));
        put<std::uint32_t>(meta, kNoRawData);
        putProperties(meta, g.properties);
        for (const auto& c : g.channels) {
            putString(meta, channelPath(g.name, c.name));
            if (c.count == 0) {
                put<std::uint32_t>(meta, kNoRawData);
            } else {
                put<std::uint32_t>(meta, kRawIndexLength);
                put<std::uint32_t>(meta, static_cast<std::uint32_t>(c.type));
                put<std::uint32_t>(meta, 1);  // dimension
                put<std::uint64_t>(meta, c.count);
                rawSize += c.count * DataType::GetLength(c.type);
            }
            putProperties(meta, c.properties);
        }
    }

    std::string leadIn;
    leadIn += "TDSm";
    put<std::uint32_t>(leadIn, kTocMetaData | kTocNewObjList | (rawSize ? kTocRawData : 0));
    put<std::int32_t>(leadIn, kVersion);
    put<std::uint64_t>(leadIn, meta.size() + rawSize);  // the next segment, from the end of the lead-in
    put<std::uint64_t>(leadIn, meta.size());            // the raw data, from the end of the lead-in
    return leadIn + meta;
}

auto SegmentBuilder::parts() -> std::vector<Part> {
    std::vector<Part> out;
    if (empty())
        return out;
    m_head = head();
    out.push_back(Part{reinterpret_cast<const std::uint8_t*>(m_head.data()), m_head.size()});
    for (const auto& g : m_groups)
        for (const auto& c : g.channels)
            for (const auto& b : c.blocks)
                out.push_back(Part{b.data.get(), b.size});
    return out;
}

auto SegmentBuilder::size() -> std::uint64_t {
    std::uint64_t total = 0;
    for (const auto& p : parts())
        total += p.size;
    return total;
}

auto SegmentBuilder::write(std::ostream& out) -> void {
    for (const auto& p : parts())
        out.write(reinterpret_cast<const char*>(p.data), static_cast<std::streamsize>(p.size));
}

auto SegmentBuilder::bytes() -> std::string {
    std::ostringstream out(std::ios::binary);
    write(out);
    return out.str();
}
