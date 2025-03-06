#include "asio_common.h"
#include <cstring>
#include <limits>
#include "data_lib/neon_asm.h"

using namespace net_lib;

auto net_lib::createBuffer(const char* buffer, size_t size) -> net_buffer {
    try {
        if ((uint64_t)std::numeric_limits<size_t>::max() <= size)
            return net_buffer(nullptr);
        ;
        auto p = std::shared_ptr<uint8_t[]>(new uint8_t[static_cast<size_t>(size)]);
        memcpy_neon(p.get(), buffer, size);
        return p;
    } catch (...) {
        return net_buffer(nullptr);
    }
}

auto net_lib::createBuffer(uint64_t size) -> net_buffer {
    try {
        if ((uint64_t)std::numeric_limits<size_t>::max() <= size)
            return net_buffer(nullptr);
        ;
        auto p = std::shared_ptr<uint8_t[]>(new uint8_t[static_cast<size_t>(size)]);
        return p;
    } catch (...) {
        return net_buffer(nullptr);
        ;
    }
}
