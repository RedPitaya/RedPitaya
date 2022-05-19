#ifndef WRITER_LIB_WQUEUE_H
#define WRITER_LIB_WQUEUE_H

#include <mutex>
#include <iostream>
#include <list>

class Queue
{
    public:
        auto queueSize() -> long;

    protected:
        Queue();
        ~Queue();

        auto pushQueue(std::iostream* buffer) -> void;
        auto popQueue() -> std::iostream*;

        uint64_t m_useMemory;

    private:
        std::list<std::iostream*> m_queue;
        std::mutex m_mutex;
};

#endif
