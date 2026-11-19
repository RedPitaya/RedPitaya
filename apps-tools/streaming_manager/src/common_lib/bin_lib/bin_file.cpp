#include "bin_file.h"

#include <algorithm>
#include <cstring>

using namespace bin_lib;

auto bin_lib::fileHeader(const FileInfo& info) -> std::vector<uint8_t> {
    FileHeader h;
    std::memset(&h, 0, sizeof(h));
    std::memcpy(h.magic, FILE_MAGIC, sizeof(h.magic));
    h.version = FORMAT_VERSION;
    h.headerSize = static_cast<uint16_t>(sizeof(FileHeader) + info.channels.size() * sizeof(ChannelEntry));
    h.source = static_cast<uint8_t>(info.source);
    h.channels = static_cast<uint8_t>(info.channels.size());
    h.flags = info.flags;
    h.clock = info.clock;
    h.decimation = info.decimation;
    h.startTime = info.startTime;

    std::vector<uint8_t> out(h.headerSize, 0);
    std::memcpy(out.data(), &h, sizeof(h));
    for (size_t i = 0; i < info.channels.size(); i++) {
        ChannelEntry c;
        std::memset(&c, 0, sizeof(c));
        c.id = info.channels[i].id;
        c.sampleType = static_cast<uint8_t>(info.channels[i].type);
        c.flags = info.channels[i].flags;
        c.adcBits = info.channels[i].adcBits;
        std::memcpy(out.data() + sizeof(FileHeader) + i * sizeof(ChannelEntry), &c, sizeof(c));
    }
    return out;
}

auto bin_lib::blockSize(uint32_t dataSize) -> uint64_t {
    return sizeof(BlockHeader) + padded(dataSize);
}

auto bin_lib::appendBlock(std::vector<uint8_t>& out, const BlockInfo& block, const void* data, uint32_t dataSize) -> void {
    BlockHeader h;
    std::memset(&h, 0, sizeof(h));
    h.magic = BLOCK_MAGIC;
    h.channel = block.channel;
    h.flags = block.flags;
    h.dataSize = dataSize;
    h.triggerCount = block.triggerCount;
    h.samples = block.samples;
    h.lost = block.lost;
    h.wait = block.wait;
    h.position = block.position;
    const size_t at = out.size();
    out.resize(at + blockSize(dataSize), 0);
    std::memcpy(out.data() + at, &h, sizeof(h));
    if (dataSize)
        std::memcpy(out.data() + at + sizeof(h), data, dataSize);
}

auto Timeline::next(uint8_t channel, uint64_t lost, uint64_t wait, uint64_t samples) -> uint64_t {
    if (m_end.size() <= channel)
        m_end.resize(channel + 1, 0);
    const uint64_t position = m_end[channel] + lost + wait;
    m_end[channel] = position + samples;
    return position;
}

auto Timeline::reset() -> void {
    m_end.clear();
}

auto FileReader::open(const std::string& path) -> bool {
    close();
    m_file.open(path, std::ios::binary);
    if (!m_file) {
        m_error = "cannot open " + path;
        return false;
    }
    m_file.seekg(0, std::ios::end);
    m_end = static_cast<uint64_t>(m_file.tellg());
    m_file.seekg(0);

    FileHeader h;
    if (m_end < sizeof(h) || !m_file.read(reinterpret_cast<char*>(&h), sizeof(h)) || std::memcmp(h.magic, FILE_MAGIC, sizeof(h.magic)) != 0) {
        m_error = "not a BIN stream file";
        close();
        return false;
    }
    if (h.version != FORMAT_VERSION) {
        m_error = "BIN stream file version " + std::to_string(h.version) + ", this reader knows " + std::to_string(FORMAT_VERSION);
        close();
        return false;
    }
    if (h.headerSize != sizeof(FileHeader) + h.channels * sizeof(ChannelEntry) || h.headerSize > m_end) {
        m_error = "broken BIN file header";
        close();
        return false;
    }
    m_info = FileInfo();
    m_info.source = static_cast<Source>(h.source);
    m_info.flags = h.flags;
    m_info.clock = h.clock;
    m_info.decimation = h.decimation;
    m_info.startTime = h.startTime;
    for (uint8_t i = 0; i < h.channels; i++) {
        ChannelEntry c;
        if (!m_file.read(reinterpret_cast<char*>(&c), sizeof(c))) {
            m_error = "broken BIN channel table";
            close();
            return false;
        }
        ChannelInfo ci;
        ci.id = c.id;
        ci.type = static_cast<SampleType>(c.sampleType);
        ci.flags = c.flags;
        ci.adcBits = c.adcBits;
        m_info.channels.push_back(ci);
    }
    m_firstBlock = h.headerSize;
    if (m_headerOnly) {
        m_pieces.assign(m_info.channels.size(), {});
        rewind();
        return true;
    }
    if (!index()) {
        m_error = "stopped";
        close();
        return false;
    }
    rewind();
    return true;
}

auto FileReader::openHeader(const std::string& path) -> bool {
    m_headerOnly = true;
    const bool ok = open(path);
    m_headerOnly = false;
    return ok;
}

auto FileReader::close() -> void {
    if (m_file.is_open())
        m_file.close();
    m_file.clear();
    m_info = FileInfo();
    m_pieces.clear();
    m_firstBlock = m_next = m_end = 0;
}

auto FileReader::error() const -> const std::string& {
    return m_error;
}

auto FileReader::info() const -> const FileInfo& {
    return m_info;
}

auto FileReader::rewind() -> void {
    m_next = m_firstBlock;
}

auto FileReader::nextBlock(BlockInfo& block, std::vector<uint8_t>& data) -> bool {
    BlockHeader h;
    if (m_next + sizeof(h) > m_end)
        return false;
    m_file.clear();
    m_file.seekg(static_cast<std::streamoff>(m_next));
    if (!m_file.read(reinterpret_cast<char*>(&h), sizeof(h)) || h.magic != BLOCK_MAGIC || h.channel >= m_info.channels.size())
        return false;
    if (m_next + blockSize(h.dataSize) > m_end)
        return false;  // cut short: the end of the recording
    data.resize(h.dataSize);
    if (h.dataSize && !m_file.read(reinterpret_cast<char*>(data.data()), h.dataSize))
        return false;
    block.channel = h.channel;
    block.flags = h.flags;
    block.triggerCount = h.triggerCount;
    block.samples = h.samples;
    block.lost = h.lost;
    block.wait = h.wait;
    block.position = h.position;
    block.dataSize = h.dataSize;
    m_next += blockSize(h.dataSize);
    return true;
}

// The pieces of the fixed size channels from the block headers, the data is not read
auto FileReader::index() -> bool {
    m_pieces.assign(m_info.channels.size(), {});
    std::vector<uint64_t> line[2];
    line[0].assign(m_info.channels.size(), 0);
    line[1].assign(m_info.channels.size(), 0);
    uint64_t at = m_firstBlock;
    uint64_t blocks = 0;
    BlockHeader h;
    while (at + sizeof(h) <= m_end) {
        m_file.clear();
        m_file.seekg(static_cast<std::streamoff>(at));
        if (!m_file.read(reinterpret_cast<char*>(&h), sizeof(h)) || h.magic != BLOCK_MAGIC || h.channel >= m_info.channels.size())
            break;
        if (at + blockSize(h.dataSize) > m_end)
            break;
        const auto& ch = m_info.channels[h.channel];
        const uint32_t bytes = sampleBytes(ch.type);
        if (bytes && !(ch.flags & CHANNEL_FLAG_RLE)) {
            Piece p;
            p.gap = h.lost + h.wait;
            p.lost = h.lost;
            p.samples = h.dataSize / bytes;
            p.offset = at + sizeof(h);
            p.start[0] = line[0][h.channel];
            p.start[1] = line[1][h.channel];
            line[0][h.channel] += p.samples;
            line[1][h.channel] += p.gap + p.samples;
            m_pieces[h.channel].push_back(p);
        }
        at += blockSize(h.dataSize);
        // now and then: a block header is read a few thousand times a second from a disk
        if (m_progress && (blocks++ & 255) == 0 && !m_progress(static_cast<double>(at) / static_cast<double>(m_end)))
            return false;
    }
    return true;
}

auto FileReader::gaps(uint8_t channel) const -> std::vector<Gap> {
    std::vector<Gap> r;
    if (channel >= m_pieces.size())
        return r;
    for (const auto& p : m_pieces[channel])
        if (p.gap)
            r.push_back(Gap{p.start[1], p.lost, p.gap - p.lost});
    return r;
}

auto FileReader::samples(uint8_t channel, bool gaps) const -> uint64_t {
    if (channel >= m_pieces.size() || m_pieces[channel].empty())
        return 0;
    const auto& last = m_pieces[channel].back();
    return gaps ? last.start[1] + last.gap + last.samples : last.start[0] + last.samples;
}

auto FileReader::read(uint8_t channel, uint64_t position, uint64_t n, void* dst, bool gaps) -> uint64_t {
    if (channel >= m_pieces.size() || n == 0)
        return 0;
    const auto& pieces = m_pieces[channel];
    const uint32_t bytes = sampleBytes(m_info.channels[channel].type);
    const int line = gaps ? 1 : 0;
    // the piece whose span (with its gap on the gap line) holds position
    auto it = std::upper_bound(pieces.begin(), pieces.end(), position, [line](uint64_t pos, const Piece& p) { return pos < p.start[line]; });
    if (it == pieces.begin())
        return 0;
    size_t i = static_cast<size_t>(it - pieces.begin()) - 1;
    auto* out = static_cast<uint8_t*>(dst);
    uint64_t done = 0;
    while (done < n && i < pieces.size()) {
        const auto& p = pieces[i];
        uint64_t at = position + done - p.start[line];  // inside the span of the piece
        if (gaps) {
            if (at < p.gap) {
                const uint64_t zeros = std::min(n - done, p.gap - at);
                std::memset(out + done * bytes, 0, zeros * bytes);
                done += zeros;
                continue;
            }
            at -= p.gap;
        }
        if (at >= p.samples) {
            i++;
            continue;
        }
        const uint64_t take = std::min(n - done, p.samples - at);
        m_file.clear();
        m_file.seekg(static_cast<std::streamoff>(p.offset + at * bytes));
        if (!m_file.read(reinterpret_cast<char*>(out + done * bytes), static_cast<std::streamsize>(take * bytes)))
            break;
        done += take;
    }
    return done;
}
