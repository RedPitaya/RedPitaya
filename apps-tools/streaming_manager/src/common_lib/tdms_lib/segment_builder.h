#ifndef TDMS_LIB_SEGMENT_BUILDER_H
#define TDMS_LIB_SEGMENT_BUILDER_H

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "value.h"

namespace TDMS {

/**
 * One TDMS segment: properties of the file, of groups and of channels, and channel samples.
 *
 * A file is a sequence of segments; a recorder builds one per buffer and hands it to FileWriter
 * (or write()s it to any stream). The segment lists all its objects (the new object list flag is
 * set), so a channel without samples in this segment simply has none here. A property written
 * once stays with its object for the rest of the file; writing it again in a later segment
 * replaces the value. Groups of the channels are written without being declared.
 *
 * Samples are either referenced or copied, and the name says which:
 *   addSamples  keeps a shared_ptr to the caller's memory, the samples go to the file from there;
 *   copySamples copies them into the builder (for small data, or memory that is reused at once).
 *
 *     SegmentBuilder s;
 *     s.setProperty("device", "STEMlab 125-14");
 *     s.setChannelProperty("Group", "ch1", "unit_string", "V");
 *     s.addSamples("Group", "ch1", buffer);        // std::shared_ptr<std::vector<int16_t>> -> Integer16
 *     writer.write(s);
 *
 * Not thread safe: one builder is filled by one thread at a time.
 */
class SegmentBuilder {
   public:
    auto setProperty(const std::string& key, const Value& value) -> void;  // of the file
    auto setGroupProperty(const std::string& group, const std::string& key, const Value& value) -> void;
    auto setChannelProperty(const std::string& group, const std::string& channel, const std::string& key, const Value& value) -> void;

    // Samples of a fixed width type, appended to what the channel already has in this segment;
    // one type per channel and segment, std::invalid_argument otherwise.

    // By reference: the builder keeps `data` alive and writes the samples from it, so the memory
    // must not change until the segment is written. Any owner fits through the aliasing
    // constructor: std::shared_ptr<const std::uint8_t[]>(owner, pointer).
    auto addSamples(const std::string& group, const std::string& channel, TDMSType type, std::shared_ptr<const std::uint8_t[]> data, std::uint64_t count) -> void;
    // A shared std::vector<T> (or const std::vector<T>) by reference; the type follows T
    template <typename V, typename T = typename V::value_type>
    auto addSamples(const std::string& group, const std::string& channel, std::shared_ptr<V> data) -> void {
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(data->data());
        const std::uint64_t count = data->size();
        addSamples(group, channel, typeOf<std::remove_cv_t<T>>(), std::shared_ptr<const std::uint8_t[]>(std::move(data), bytes), count);
    }

    // By copy: the samples are copied into the builder now
    auto copySamples(const std::string& group, const std::string& channel, TDMSType type, const void* data, std::uint64_t count) -> void;
    template <typename T>
    auto copySamples(const std::string& group, const std::string& channel, const T* data, std::uint64_t count) -> void {
        copySamples(group, channel, typeOf<T>(), static_cast<const void*>(data), count);
    }
    template <typename T>
    auto copySamples(const std::string& group, const std::string& channel, const std::vector<T>& data) -> void {
        copySamples(group, channel, data.data(), data.size());
    }

    auto empty() const -> bool;
    auto clear() -> void;

    // The segment as pieces to write one after another: the lead-in with the metadata (a few
    // hundred bytes, held by the builder), then the sample blocks where they are. A recorder hands
    // these to the file thread and nothing is copied into an intermediate stream; they stay valid
    // while the builder lives and is not changed. Empty for an empty builder.
    struct Part {
        const std::uint8_t* data = nullptr;
        std::uint64_t size = 0;
    };
    auto parts() -> std::vector<Part>;
    auto size() -> std::uint64_t;  // bytes of the whole segment

    // Appends the parts to a stream; an empty builder writes nothing
    auto write(std::ostream& out) -> void;
    // The segment in one string (a copy; for tests and small segments)
    auto bytes() -> std::string;

   private:
    struct Block {
        std::shared_ptr<const std::uint8_t[]> data;
        std::uint64_t size = 0;
    };
    struct Channel {
        std::string name;
        Properties properties;
        TDMSType type = TDMSType::Empty;
        std::uint64_t count = 0;
        std::vector<Block> blocks;
    };
    struct Group {
        std::string name;
        Properties properties;
        std::vector<Channel> channels;
    };

    auto group(const std::string& name) -> Group&;
    auto channel(const std::string& group, const std::string& name) -> Channel&;

    auto head() const -> std::string;

    Properties m_properties;
    std::vector<Group> m_groups;  // in the order of first use, as they are written
    std::string m_head;           // the lead-in and the metadata of the last parts()
};

}  // namespace TDMS

#endif
