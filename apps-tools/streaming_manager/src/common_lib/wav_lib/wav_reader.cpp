#include "wav_reader.h"
#include <cstring>

namespace {

constexpr uint16_t kFormatPCM = 1;
constexpr uint16_t kFormatFloat = 3;
constexpr uint16_t kFormatExtensible = 0xFFFE;

template <typename T>
auto le(const char* p) -> T {
    T v;
    std::memcpy(&v, p, sizeof(T));
    return v;
}

}  // namespace

CWaveReader::CWaveReader() {}

CWaveReader::~CWaveReader() {
    if (m_read_fs.is_open())
        m_read_fs.close();
}

auto CWaveReader::frames() const -> uint64_t {
    return m_blockAlign ? m_dataSize / m_blockAlign : 0;
}

auto CWaveReader::openFile(std::string fileName) -> bool {
    m_error.clear();
    m_format = Format::OTHER;
    m_channels = m_bits = 0;
    m_rate = m_blockAlign = 0;
    m_dataOffset = m_dataSize = 0;
    if (m_read_fs.is_open())
        m_read_fs.close();
    m_read_fs.clear();
    m_read_fs.open(fileName, std::ios::binary);
    if (!m_read_fs.is_open()) {
        m_error = "cannot open " + fileName;
        return false;
    }
    m_read_fs.seekg(0, std::ios::end);
    const uint64_t length = static_cast<uint64_t>(m_read_fs.tellg());
    m_read_fs.seekg(0, std::ios::beg);

    char riff[12];
    if (!m_read_fs.read(riff, sizeof(riff)) || std::memcmp(riff, "RIFF", 4) != 0 || std::memcmp(riff + 8, "WAVE", 4) != 0) {
        m_error = "not a RIFF/WAVE file";
        return false;
    }
    bool haveFormat = false;
    uint64_t pos = 12;
    while (pos + 8 <= length) {
        char head[8];
        m_read_fs.clear();
        m_read_fs.seekg(static_cast<std::streamoff>(pos));
        if (!m_read_fs.read(head, sizeof(head)))
            break;
        const uint64_t size = le<uint32_t>(head + 4);
        const uint64_t body = pos + 8;
        if (std::memcmp(head, "fmt ", 4) == 0) {
            char fmt[40] = {};
            const uint64_t n = size < sizeof(fmt) ? size : sizeof(fmt);
            if (size < 16 || !m_read_fs.read(fmt, static_cast<std::streamsize>(n))) {
                m_error = "a broken fmt chunk";
                return false;
            }
            uint16_t tag = le<uint16_t>(fmt);
            m_channels = le<uint16_t>(fmt + 2);
            m_rate = le<uint32_t>(fmt + 4);
            m_blockAlign = le<uint16_t>(fmt + 12);
            m_bits = le<uint16_t>(fmt + 14);
            if (tag == kFormatExtensible && size >= 40)
                tag = le<uint16_t>(fmt + 24);  // the first two bytes of the subformat GUID
            m_format = tag == kFormatPCM ? Format::PCM : tag == kFormatFloat ? Format::FLOAT : Format::OTHER;
            haveFormat = true;
        } else if (std::memcmp(head, "data", 4) == 0) {
            if (!haveFormat) {
                m_error = "the data chunk comes before fmt";
                return false;
            }
            m_dataOffset = body;
            // A writer that never finished leaves 0 or 0xFFFFFFFF; the samples then last to the end
            m_dataSize = (size == 0 || size == 0xFFFFFFFFu || body + size > length) ? length - body : size;
            if (m_blockAlign == 0 || m_blockAlign != static_cast<uint32_t>(m_channels) * ((m_bits + 7u) / 8u)) {
                m_error = "an inconsistent fmt chunk";
                return false;
            }
            return true;
        }
        pos = body + size + (size & 1);  // chunks are word aligned
    }
    m_error = haveFormat ? "no data chunk" : "no fmt chunk";
    return false;
}

auto CWaveReader::readFrames(uint64_t first, uint64_t count, void* dst) -> uint64_t {
    const uint64_t total = frames();
    if (!m_read_fs.is_open() || first >= total)
        return 0;
    if (count > total - first)
        count = total - first;
    m_read_fs.clear();
    m_read_fs.seekg(static_cast<std::streamoff>(m_dataOffset + first * m_blockAlign));
    m_read_fs.read(static_cast<char*>(dst), static_cast<std::streamsize>(count * m_blockAlign));
    return static_cast<uint64_t>(m_read_fs.gcount()) / m_blockAlign;
}
