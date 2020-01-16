//
// Created by user on 03.04.19.
//
#include "asio.hpp"

#ifndef PROJECT_SHARED_BUFFER_H
#define PROJECT_SHARED_BUFFER_H

class shared_const_buffer
{
public:
    // Construct from a std::string.
    explicit shared_const_buffer(const std::string& data)
            : data_(new std::vector<char>(data.begin(), data.end())),
              buffer_(asio::buffer(*data_))
    {
    }

    // Implement the ConstBufferSequence requirements.
    typedef asio::const_buffer value_type;
    typedef const asio::const_buffer* const_iterator;
    const asio::const_buffer* begin() const { return &buffer_; }
    const asio::const_buffer* end() const { return &buffer_ + 1; }

private:
    std::shared_ptr<std::vector<char> > data_;
    asio::const_buffer buffer_;
};

#endif //PROJECT_SHARED_BUFFER_H
