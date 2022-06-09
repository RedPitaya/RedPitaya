#include "w_queue.h"



Queue::Queue():
    m_useMemory(0){

}

Queue::~Queue(){
}

auto Queue::pushQueue(std::iostream* buffer) -> void{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(buffer);
    buffer->seekg(0, std::ios::end);
    auto Length = buffer->tellg();
    m_useMemory += Length;
}

auto Queue::popQueue() -> std::iostream*{
    const std::lock_guard<std::mutex> lock(m_mutex);
    std::iostream* buffer = m_queue.front();
    if (buffer != nullptr){
        m_queue.pop_front();
        buffer->seekg(0, std::ios::end);
        auto Length = buffer->tellg();
        m_useMemory -= Length;
        buffer->seekg(0, std::ios::beg);
    }
    return buffer;
}

auto Queue::queueSize() -> long{
    const std::lock_guard<std::mutex> lock(m_mutex);
    long size = m_queue.size();
    return size;
}

