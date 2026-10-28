#include "file_reader.h"

#include <algorithm>
#include <cstring>
#include <fstream>

#include "path.h"

using namespace TDMS;

namespace {

constexpr std::uint32_t kTocMetaData = 1 << 1;
constexpr std::uint32_t kTocNewObjList = 1 << 2;
constexpr std::uint32_t kTocRawData = 1 << 3;
constexpr std::uint32_t kTocInterleaved = 1 << 5;
constexpr std::uint32_t kTocBigEndian = 1 << 6;
constexpr std::uint32_t kTocDaqMx = 1 << 7;
constexpr std::int64_t kLeadIn = 28;
constexpr std::uint32_t kNoRawData = 0xFFFFFFFF;
constexpr std::uint32_t kSameRawIndex = 0;
constexpr std::uint64_t kMaxBytes = 1ULL << 62;  // no size in a file can be larger; keeps the sums from overflowing

// The width of a sample or property of a fixed width type, 0 for the others
auto fixedWidth(TDMSType type) -> std::uint32_t {
    switch (type) {
        case TDMSType::Integer8:
        case TDMSType::UnsignedInteger8:
        case TDMSType::Boolean:
            return 1;
        case TDMSType::Integer16:
        case TDMSType::UnsignedInteger16:
            return 2;
        case TDMSType::Integer32:
        case TDMSType::UnsignedInteger32:
        case TDMSType::SingleFloat:
        case TDMSType::SingleFloatWithUnit:
            return 4;
        case TDMSType::Integer64:
        case TDMSType::UnsignedInteger64:
        case TDMSType::DoubleFloat:
        case TDMSType::DoubleFloatWithUnit:
            return 8;
        case TDMSType::TimeStamp:
            return 16;
        default:
            return 0;
    }
}

// Sequential reads of a lead-in or a metadata section, loaded at once
class Cursor {
   public:
    auto load(std::istream& in, std::int64_t pos, std::int64_t size) -> bool {
        m_data.resize(static_cast<std::size_t>(size));
        m_pos = 0;
        m_end = size;
        in.clear();
        in.seekg(pos);
        in.read(m_data.data(), size);
        return in.gcount() == size;
    }

    auto bytes(void* out, std::int64_t size) -> bool {
        if (size < 0 || size > m_end - m_pos)
            return false;
        std::memcpy(out, m_data.data() + m_pos, static_cast<std::size_t>(size));
        m_pos += size;
        return true;
    }
    template <typename T>
    auto get(T& value) -> bool {
        return bytes(&value, sizeof(T));
    }
    auto string(std::string& out) -> bool {
        std::uint32_t length = 0;
        if (!get(length) || length > m_end - m_pos)
            return false;
        out.resize(length);
        return length == 0 || bytes(&out[0], length);
    }

   private:
    std::vector<char> m_data;
    std::int64_t m_pos = 0;
    std::int64_t m_end = 0;
};

const Properties kNoProperties;

}  // namespace

FileReader::FileReader() = default;
FileReader::~FileReader() = default;

auto FileReader::open(const std::string& path) -> bool {
    auto file = std::make_unique<std::ifstream>(path, std::ios::binary);
    if (!file->is_open()) {
        close();
        return fail("cannot open " + path);
    }
    return open(std::move(file));
}

auto FileReader::open(std::unique_ptr<std::istream> stream) -> bool {
    close();
    if (!stream)
        return fail("no stream");
    m_stream = std::move(stream);
    m_stream->seekg(0, std::ios::end);
    m_size = static_cast<std::int64_t>(m_stream->tellg());
    if (m_size < 0) {
        close();
        return fail("cannot get the size of the file");
    }
    if (!parse()) {
        const std::string message = m_error;
        close();
        return fail(message);
    }
    return true;
}

auto FileReader::close() -> void {
    m_stream.reset();
    m_size = 0;
    m_error.clear();
    m_objects.clear();
    m_byPath.clear();
    m_active.clear();
    m_activeHasData.clear();
    m_truncated = false;
}

auto FileReader::truncated() const -> bool {
    return m_truncated;
}

auto FileReader::isOpen() const -> bool {
    return m_stream != nullptr;
}

auto FileReader::error() const -> const std::string& {
    return m_error;
}

auto FileReader::fail(const std::string& message) -> bool {
    m_error = message;
    return false;
}

auto FileReader::object(const std::string& path, const std::vector<std::string>& names) -> std::size_t {
    auto it = m_byPath.find(path);
    if (it != m_byPath.end())
        return it->second;
    Object o;
    o.names = names;
    if (names.size() >= 1)
        o.info.group = names[0];
    if (names.size() == 2)
        o.info.name = names[1];
    m_objects.push_back(std::move(o));
    m_byPath[path] = m_objects.size() - 1;
    return m_objects.size() - 1;
}

auto FileReader::parse() -> bool {
    // The file object is always there, with or without properties
    object("/", {});
    if (m_size < kLeadIn)
        return fail("not a TDMS file: shorter than a segment lead-in");
    std::int64_t offset = 0;
    while (offset >= 0 && offset < m_size) {
        if (m_size - offset < kLeadIn)
            break;  // bytes after the last segment
        std::int64_t next = -1;
        if (!readSegment(offset, next))
            return false;
        offset = next;
    }
    return true;
}

auto FileReader::readSegment(std::int64_t offset, std::int64_t& next) -> bool {
    char tag[4];
    std::uint32_t toc = 0;
    std::int32_t version = 0;
    std::uint64_t nextOffset = 0;
    std::uint64_t rawOffset = 0;
    Cursor leadIn;
    if (!leadIn.load(*m_stream, offset, kLeadIn) || !leadIn.bytes(tag, 4) || !leadIn.get(toc) || !leadIn.get(version) || !leadIn.get(nextOffset) || !leadIn.get(rawOffset))
        return fail("cannot read the segment at " + std::to_string(offset));
    if (std::memcmp(tag, "TDSm", 4) != 0) {
        if (offset == 0)
            return fail("not a TDMS file: no TDSm tag");
        next = -1;  // not a segment: the file ends with something else
        return true;
    }
    if (toc & kTocBigEndian)
        return fail("big-endian TDMS files are not supported");
    if (toc & kTocDaqMx)
        return fail("DAQmx raw data is not supported");

    const std::int64_t body = offset + kLeadIn;
    // An incomplete segment (cut off recording) has 0xFFFF... or an offset past the end: it lasts to the end of the file
    std::int64_t end = m_size;
    next = -1;
    if (nextOffset <= static_cast<std::uint64_t>(m_size - body)) {
        end = body + static_cast<std::int64_t>(nextOffset);
        next = end;
    }
    if (rawOffset > static_cast<std::uint64_t>(end - body)) {
        // The file stops inside the metadata of its last segment (a recording cut off by a full
        // disk or a power loss): the segments before it are the file
        if (next < 0 && offset > 0) {
            m_truncated = true;
            return true;
        }
        return fail("the raw data offset of the segment at " + std::to_string(offset) + " is outside it");
    }
    const std::int64_t rawStart = body + static_cast<std::int64_t>(rawOffset);

    if (toc & kTocNewObjList) {
        m_active.clear();
        m_activeHasData.clear();
    }

    if (toc & kTocMetaData) {
        Cursor meta;
        std::uint32_t count = 0;
        // Without raw data some writers leave the raw data offset 0: the metadata then fills the segment
        const std::int64_t metaEnd = (rawOffset == 0 && !(toc & kTocRawData)) ? end : rawStart;
        if (!meta.load(*m_stream, body, metaEnd - body) || !meta.get(count))
            return fail("cannot read the object count of the segment at " + std::to_string(offset));
        for (std::uint32_t i = 0; i < count; i++) {
            std::string path;
            std::uint32_t index = 0;
            if (!meta.string(path) || !meta.get(index))
                return fail("cannot read object " + std::to_string(i) + " of the segment at " + std::to_string(offset));
            std::vector<std::string> names;
            if (!parsePath(path, names))
                return fail("not an object path: " + path);
            const std::size_t id = object(path, names);
            Object& o = m_objects[id];

            bool hasData = false;
            if (index == kNoRawData) {
                hasData = false;
            } else if (index == kSameRawIndex) {
                if (!o.hasIndex)
                    return fail(path + " reuses a raw data index it never had");
                hasData = true;
            } else if (index == 20 || index == 28) {
                std::uint32_t type = 0;
                std::uint32_t dimension = 0;
                std::uint64_t values = 0;
                std::uint64_t bytes = 0;
                if (!meta.get(type) || !meta.get(dimension) || !meta.get(values))
                    return fail("cannot read the raw data index of " + path);
                if (index == 28 && !meta.get(bytes))
                    return fail("cannot read the raw data index of " + path);
                const auto t = static_cast<TDMSType>(type);
                if (dimension != 1)
                    return fail(path + " has data of dimension " + std::to_string(dimension));
                if (t == TDMSType::String) {
                    if (index != 28)
                        return fail(path + " is a string channel without its size");
                } else {
                    const std::uint32_t width = fixedWidth(t);
                    if (width == 0)
                        return fail(path + " has data of an unsupported type " + std::to_string(type));
                    if (values > kMaxBytes / width)
                        return fail(path + " has an impossible number of values");
                    bytes = values * width;
                }
                if (bytes > kMaxBytes)
                    return fail(path + " has an impossible data size");
                o.hasIndex = true;
                o.indexType = t;
                o.indexCount = values;
                o.indexBytes = bytes;
                hasData = true;
            } else {
                return fail(path + " has an unsupported raw data index " + std::to_string(index));
            }
            if (hasData && names.size() != 2)
                return fail(path + " is not a channel but has raw data");

            std::uint32_t propertyCount = 0;
            if (!meta.get(propertyCount))
                return fail("cannot read the properties of " + path);
            for (std::uint32_t p = 0; p < propertyCount; p++) {
                std::string key;
                std::uint32_t type = 0;
                if (!meta.string(key) || !meta.get(type))
                    return fail("cannot read a property of " + path);
                const auto t = static_cast<TDMSType>(type);
                if (t == TDMSType::String) {
                    std::string text;
                    if (!meta.string(text))
                        return fail("cannot read property " + key + " of " + path);
                    o.info.properties[key] = Value(text);
                } else {
                    const std::uint32_t width = fixedWidth(t);
                    std::uint8_t payload[16];
                    if (width == 0)
                        return fail("property " + key + " of " + path + " has an unsupported type " + std::to_string(type));
                    if (!meta.bytes(payload, width))
                        return fail("cannot read property " + key + " of " + path);
                    o.info.properties[key] = Value::fromBytes(t, payload, width);
                }
            }

            auto at = std::find(m_active.begin(), m_active.end(), id);
            if (at == m_active.end()) {
                m_active.push_back(id);
                m_activeHasData.push_back(hasData);
            } else {
                m_activeHasData[static_cast<std::size_t>(at - m_active.begin())] = hasData;
            }
        }
    }

    if (!(toc & kTocRawData))
        return true;

    // The channels with samples in this segment, in order
    std::vector<std::size_t> layout;
    for (std::size_t i = 0; i < m_active.size(); i++) {
        const Object& o = m_objects[m_active[i]];
        if (m_activeHasData[i] && o.hasIndex && o.indexCount > 0)
            layout.push_back(m_active[i]);
    }
    if (layout.empty())
        return true;
    for (auto id : layout) {
        Object& o = m_objects[id];
        if (o.info.type != TDMSType::Empty && o.info.type != o.indexType)
            return fail(channelPath(o.info.group, o.info.name) + " changes its data type");
        o.info.type = o.indexType;
    }
    const std::int64_t rawBytes = end - rawStart;

    if (toc & kTocInterleaved) {
        std::uint64_t stride = 0;
        for (auto id : layout) {
            if (m_objects[id].indexType == TDMSType::String)
                return fail("interleaved string data is not supported");
            stride += fixedWidth(m_objects[id].indexType);
        }
        const std::uint64_t rows = static_cast<std::uint64_t>(rawBytes) / stride;
        std::uint64_t position = 0;
        for (auto id : layout) {
            Object& o = m_objects[id];
            if (rows)
                o.chunks.push_back(Chunk{rawStart + static_cast<std::int64_t>(position), rows, static_cast<std::uint32_t>(stride)});
            o.info.count += rows;
            position += fixedWidth(o.indexType);
        }
        return true;
    }

    std::uint64_t chunkBytes = 0;
    for (auto id : layout)
        chunkBytes += m_objects[id].indexBytes;
    if (chunkBytes == 0)
        return fail("the segment at " + std::to_string(offset) + " has samples of no size");
    std::int64_t position = rawStart;
    while (position < end) {
        const std::int64_t left = end - position;
        if (static_cast<std::uint64_t>(left) >= chunkBytes) {
            for (auto id : layout) {
                Object& o = m_objects[id];
                o.chunks.push_back(Chunk{position, o.indexCount, 0});
                o.info.count += o.indexCount;
                position += static_cast<std::int64_t>(o.indexBytes);
            }
            continue;
        }
        // A cut off last chunk: every channel keeps the whole samples that are there
        std::int64_t available = left;
        for (auto id : layout) {
            Object& o = m_objects[id];
            if (o.indexType == TDMSType::String || available <= 0)
                break;
            const std::uint32_t width = fixedWidth(o.indexType);
            const std::uint64_t values = std::min<std::uint64_t>(o.indexCount, static_cast<std::uint64_t>(available) / width);
            if (values) {
                o.chunks.push_back(Chunk{position, values, 0});
                o.info.count += values;
            }
            position += static_cast<std::int64_t>(o.indexBytes);
            available -= static_cast<std::int64_t>(o.indexBytes);
        }
        break;
    }
    return true;
}

auto FileReader::properties() const -> const Properties& {
    return m_objects.empty() ? kNoProperties : m_objects[0].info.properties;
}

auto FileReader::groups() const -> std::vector<std::string> {
    std::vector<std::string> out;
    for (const auto& o : m_objects) {
        if (o.names.empty())
            continue;
        if (std::find(out.begin(), out.end(), o.names[0]) == out.end())
            out.push_back(o.names[0]);
    }
    return out;
}

auto FileReader::groupProperties(const std::string& group) const -> const Properties& {
    auto it = m_byPath.find(groupPath(group));
    return it == m_byPath.end() ? kNoProperties : m_objects[it->second].info.properties;
}

auto FileReader::channels() const -> std::vector<ChannelInfo> {
    std::vector<ChannelInfo> out;
    for (const auto& o : m_objects)
        if (o.names.size() == 2)
            out.push_back(o.info);
    return out;
}

auto FileReader::channels(const std::string& group) const -> std::vector<ChannelInfo> {
    std::vector<ChannelInfo> out;
    for (const auto& o : m_objects)
        if (o.names.size() == 2 && o.names[0] == group)
            out.push_back(o.info);
    return out;
}

auto FileReader::find(const std::string& group, const std::string& name) const -> const Object* {
    auto it = m_byPath.find(channelPath(group, name));
    return it == m_byPath.end() ? nullptr : &m_objects[it->second];
}

auto FileReader::channel(const std::string& group, const std::string& name) const -> const ChannelInfo* {
    const Object* o = find(group, name);
    return o ? &o->info : nullptr;
}

auto FileReader::read(const std::string& group, const std::string& name, std::uint64_t first, std::uint64_t count, std::vector<std::uint8_t>& out) -> std::uint64_t {
    const Object* o = find(group, name);
    if (o == nullptr || first >= o->info.count)
        return 0;
    const std::uint32_t width = fixedWidth(o->info.type);
    const std::size_t at = out.size();
    out.resize(at + static_cast<std::size_t>(std::min(count, o->info.count - first) * width));
    const std::uint64_t n = read(group, name, first, count, static_cast<void*>(out.data() + at));
    out.resize(at + static_cast<std::size_t>(n * width));
    return n;
}

auto FileReader::read(const std::string& group, const std::string& name, std::uint64_t first, std::uint64_t count, void* dst) -> std::uint64_t {
    const Object* o = find(group, name);
    if (o == nullptr || !m_stream || dst == nullptr)
        return 0;
    const std::uint32_t width = fixedWidth(o->info.type);
    if (width == 0)
        return 0;
    auto* out = static_cast<std::uint8_t*>(dst);
    std::uint64_t done = 0;
    std::uint64_t skip = first;
    for (const auto& chunk : o->chunks) {
        if (done == count)
            break;
        if (skip >= chunk.count) {
            skip -= chunk.count;
            continue;
        }
        const std::uint64_t n = std::min(chunk.count - skip, count - done);
        std::uint8_t* at = out + done * width;
        m_stream->clear();
        if (chunk.stride == 0) {
            m_stream->seekg(chunk.offset + static_cast<std::int64_t>(skip * width));
            m_stream->read(reinterpret_cast<char*>(at), static_cast<std::streamsize>(n * width));
            const std::uint64_t got = static_cast<std::uint64_t>(m_stream->gcount()) / width;
            if (got != n) {
                done += got;
                break;
            }
        } else {
            // Interleaved: read rows in blocks and keep this channel's column
            const std::uint64_t rowsPerBlock = std::max<std::uint64_t>(1, (1u << 20) / chunk.stride);
            std::vector<char> rows;
            std::uint64_t got = 0;
            while (got < n) {
                const std::uint64_t r = std::min(rowsPerBlock, n - got);
                const std::uint64_t size = (r - 1) * chunk.stride + width;
                rows.resize(static_cast<std::size_t>(size));
                m_stream->seekg(chunk.offset + static_cast<std::int64_t>((skip + got) * chunk.stride));
                m_stream->read(rows.data(), static_cast<std::streamsize>(size));
                if (static_cast<std::uint64_t>(m_stream->gcount()) != size)
                    break;
                for (std::uint64_t k = 0; k < r; k++)
                    std::memcpy(at + (got + k) * width, rows.data() + k * chunk.stride, width);
                got += r;
            }
            if (got < n) {
                done += got;
                break;
            }
        }
        done += n;
        skip = 0;
    }
    return done;
}
