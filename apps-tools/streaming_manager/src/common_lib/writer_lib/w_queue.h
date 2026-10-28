#ifndef WRITER_LIB_WQUEUE_H
#define WRITER_LIB_WQUEUE_H

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <list>
#include <mutex>

#include "file_chunk.h"

// The chunks waiting for the file thread, with the memory they hold
class Queue {
   public:
    auto queueSize() -> long;
    auto queueMemory() -> std::uint64_t;

   protected:
    Queue();
    ~Queue();

    // Takes the chunk if the queue holds less than `limit` bytes; false and the chunk dropped otherwise
    auto pushQueue(FileChunk::Ptr chunk, std::uint64_t limit) -> bool;
    // The oldest chunk, or nullptr when there is none within `wait`
    auto popQueue(std::chrono::milliseconds wait = std::chrono::milliseconds(0)) -> FileChunk::Ptr;

   private:
    std::list<FileChunk::Ptr> m_queue;
    std::uint64_t m_useMemory;
    std::mutex m_mutex;
    std::condition_variable m_ready;
};

#endif
