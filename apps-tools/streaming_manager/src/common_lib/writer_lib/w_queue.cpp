#include "w_queue.h"

Queue::Queue() : m_useMemory(0) {}

Queue::~Queue() {}

auto Queue::pushQueue(FileChunk::Ptr chunk, std::uint64_t limit) -> bool {
    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        if (m_useMemory >= limit)
            return false;
        m_useMemory += chunk->size();
        m_queue.push_back(std::move(chunk));
    }
    m_ready.notify_one();
    return true;
}

auto Queue::popQueue(std::chrono::milliseconds wait) -> FileChunk::Ptr {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty() && wait.count() > 0)
        m_ready.wait_for(lock, wait, [this] { return !m_queue.empty(); });
    if (m_queue.empty())
        return nullptr;
    FileChunk::Ptr chunk = std::move(m_queue.front());
    m_queue.pop_front();
    m_useMemory -= chunk->size();
    return chunk;
}

auto Queue::queueSize() -> long {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<long>(m_queue.size());
}

auto Queue::queueMemory() -> std::uint64_t {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_useMemory;
}
