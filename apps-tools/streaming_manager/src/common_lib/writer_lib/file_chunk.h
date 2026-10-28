#ifndef WRITER_LIB_FILE_CHUNK_H
#define WRITER_LIB_FILE_CHUNK_H

#include <cstdint>
#include <memory>
#include <ostream>
#include <vector>

/**
 * A piece of a file: blocks of memory written one after another.
 *
 * A block is either memory of someone else, kept alive through its owner until the chunk is
 * written, or bytes the chunk owns. Either way nothing is copied on the way to the file: the file
 * thread writes the blocks from where they are.
 */
class FileChunk {
   public:
    using Ptr = std::unique_ptr<FileChunk>;

    struct Block {
        const std::uint8_t* data = nullptr;
        std::uint64_t size = 0;
    };

    // Memory of `owner`, which must not change until the chunk is written
    auto add(std::shared_ptr<const void> owner, const void* data, std::uint64_t size) -> void;
    // Bytes the chunk takes over
    auto add(std::vector<std::uint8_t>&& bytes) -> void;

    auto blocks() const -> const std::vector<Block>& { return m_blocks; }
    auto size() const -> std::uint64_t { return m_size; }
    auto empty() const -> bool { return m_size == 0; }

    // The blocks one after another; false if the stream failed
    auto writeTo(std::ostream& out) const -> bool;

   private:
    std::vector<Block> m_blocks;
    std::vector<std::shared_ptr<const void>> m_owners;
    std::uint64_t m_size = 0;
};

#endif
