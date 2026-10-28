#include "file_chunk.h"

auto FileChunk::add(std::shared_ptr<const void> owner, const void* data, std::uint64_t size) -> void {
    if (size == 0)
        return;
    m_blocks.push_back(Block{static_cast<const std::uint8_t*>(data), size});
    m_owners.push_back(std::move(owner));
    m_size += size;
}

auto FileChunk::add(std::vector<std::uint8_t>&& bytes) -> void {
    if (bytes.empty())
        return;
    // The vector moves into shared ownership; its heap block, and so the pointer, stays where it is
    auto owner = std::make_shared<std::vector<std::uint8_t>>(std::move(bytes));
    const std::uint8_t* data = owner->data();
    const std::uint64_t size = owner->size();
    add(std::shared_ptr<const void>(std::move(owner)), data, size);
}

auto FileChunk::writeTo(std::ostream& out) const -> bool {
    for (const auto& b : m_blocks) {
        out.write(reinterpret_cast<const char*>(b.data), static_cast<std::streamsize>(b.size));
        if (!out)
            return false;
    }
    return true;
}
