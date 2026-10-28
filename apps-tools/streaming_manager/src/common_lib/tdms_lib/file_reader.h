#ifndef TDMS_LIB_FILE_READER_H
#define TDMS_LIB_FILE_READER_H

#include <algorithm>
#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "value.h"

namespace TDMS {

struct ChannelInfo {
    std::string group;
    std::string name;
    TDMSType type = TDMSType::Empty;  // of the samples; Empty for a channel without any
    std::uint64_t count = 0;          // samples in the whole file
    Properties properties;            // the last value of every property written in the file
};

/**
 * Reads a TDMS file by channel.
 *
 * open() walks the segments once and keeps where every channel has its samples; samples are read
 * only when asked for, one channel at a time, so a file is never loaded whole and channels that
 * are not needed cost nothing.
 *
 *     FileReader f;
 *     if (!f.open(path)) { ... f.error() ... }
 *     for (auto& c : f.channels()) { ... c.group, c.name, c.type, c.count, c.properties ... }
 *     auto ch1 = f.read<int16_t>("Group", "ch1", 0, 16384);
 *
 * Supported: little-endian files of TDMS 1.0 and 2.0, incremental metadata, interleaved data,
 * several chunks of samples in a segment, a file cut off in its last segment (the whole samples
 * that are there count; a segment cut in its metadata is left out, see truncated()). Refused
 * with an error: big-endian files and DAQmx raw data. Samples of string channels are not read;
 * such a channel is listed with its count.
 *
 * Not thread safe: reads move the position of the one stream, so a reader is used by one thread
 * at a time; open a reader per thread to read in parallel.
 */
class FileReader {
   public:
    FileReader();
    ~FileReader();
    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;

    auto open(const std::string& path) -> bool;
    auto open(std::unique_ptr<std::istream> stream) -> bool;
    auto close() -> void;
    auto isOpen() const -> bool;
    // Why open() failed
    auto error() const -> const std::string&;
    // The file ends with a segment cut off in its metadata, which is left out
    auto truncated() const -> bool;

    auto properties() const -> const Properties&;  // of the file
    auto groups() const -> std::vector<std::string>;
    auto groupProperties(const std::string& group) const -> const Properties&;
    auto channels() const -> std::vector<ChannelInfo>;  // in the order they appear in the file
    auto channels(const std::string& group) const -> std::vector<ChannelInfo>;
    auto channel(const std::string& group, const std::string& name) const -> const ChannelInfo*;

    // Reads up to `count` samples from sample `first` straight from the file into `out`, which
    // has room for count samples of the channel type (little-endian bytes); returns how many
    // samples that was, 0 for a channel that is not there or holds strings. No buffer in between,
    // so a caller reads into its own memory (a DAC block) without a copy.
    auto read(const std::string& group, const std::string& name, std::uint64_t first, std::uint64_t count, void* out) -> std::uint64_t;

    // The same appended to `out`
    auto read(const std::string& group, const std::string& name, std::uint64_t first, std::uint64_t count, std::vector<std::uint8_t>& out) -> std::uint64_t;

    // The samples as T, which has to be the C++ type of the channel type (int16_t for Integer16);
    // nothing for another T
    template <typename T>
    auto read(const std::string& group, const std::string& name, std::uint64_t first, std::uint64_t count) -> std::vector<T> {
        std::vector<T> values;
        const ChannelInfo* c = channel(group, name);
        if (c == nullptr || c->type != typeOf<T>() || first >= c->count)
            return values;
        values.resize(static_cast<std::size_t>(std::min(count, c->count - first)));
        values.resize(static_cast<std::size_t>(read(group, name, first, values.size(), static_cast<void*>(values.data()))));
        return values;
    }
    template <typename T>
    auto readAll(const std::string& group, const std::string& name) -> std::vector<T> {
        const ChannelInfo* c = channel(group, name);
        return c ? read<T>(group, name, 0, c->count) : std::vector<T>();
    }

   private:
    struct Chunk {
        std::int64_t offset = 0;   // of the first sample in the file
        std::uint64_t count = 0;
        std::uint32_t stride = 0;  // bytes from one sample to the next when interleaved, 0 if contiguous
    };
    struct Object {
        std::vector<std::string> names;  // none: the file, one: a group, two: a channel
        ChannelInfo info;
        std::vector<Chunk> chunks;
        // The raw data index of the latest segment that gave one
        bool hasIndex = false;
        TDMSType indexType = TDMSType::Empty;
        std::uint64_t indexCount = 0;
        std::uint64_t indexBytes = 0;
    };

    auto fail(const std::string& message) -> bool;
    auto parse() -> bool;
    auto readSegment(std::int64_t offset, std::int64_t& next) -> bool;
    auto object(const std::string& path, const std::vector<std::string>& names) -> std::size_t;
    auto find(const std::string& group, const std::string& name) const -> const Object*;

    std::unique_ptr<std::istream> m_stream;
    std::int64_t m_size = 0;
    std::string m_error;
    std::vector<Object> m_objects;  // in the order they appear in the file
    std::map<std::string, std::size_t> m_byPath;
    std::vector<std::size_t> m_active;  // the objects of the current raw data layout, in order
    std::vector<bool> m_activeHasData;
    bool m_truncated = false;
};

}  // namespace TDMS

#endif
