#ifndef TDMS_LIB_FILE_WRITER_H
#define TDMS_LIB_FILE_WRITER_H

#include <cstdint>
#include <fstream>
#include <string>

#include "segment_builder.h"

namespace TDMS {

/**
 * A TDMS file written segment by segment.
 *
 * write() puts the parts of a SegmentBuilder into the file one after another: the samples go
 * from the memory the builder references straight to the file, without an intermediate buffer.
 *
 *     FileWriter w;
 *     if (!w.open(path)) { ... w.error() ... }
 *     for each buffer: { SegmentBuilder s; ...; w.write(s); }
 *
 * Not thread safe: one writer is used by one thread at a time.
 */
class FileWriter {
   public:
    FileWriter() = default;
    ~FileWriter();
    FileWriter(const FileWriter&) = delete;
    FileWriter& operator=(const FileWriter&) = delete;

    // A new file, or the end of an existing one with `append`
    auto open(const std::string& path, bool append = false) -> bool;
    // Flushes and closes; false if the file could not be written completely
    auto close() -> bool;
    auto isOpen() const -> bool;
    auto error() const -> const std::string&;

    // False when the file could not take the segment (disk full); the file then ends with a
    // partial segment, which FileReader reads up to its last whole sample
    auto write(SegmentBuilder& segment) -> bool;
    auto flush() -> bool;
    auto size() const -> std::uint64_t;  // bytes in the file

   private:
    std::ofstream m_file;
    std::string m_path;
    std::string m_error;
    std::uint64_t m_size = 0;
};

}  // namespace TDMS

#endif
