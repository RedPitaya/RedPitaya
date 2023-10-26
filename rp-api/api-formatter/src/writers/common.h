#ifndef __RP_COMMON_H__
#define __RP_COMMON_H__

#include <map>
#include "rp_formatter.h"

namespace rp_formatter_api{

struct SBufferPack{
    std::map<rp_channel_t,rp_bits_t> m_bits = {};
    std::map<rp_channel_t,size_t> m_samplesCount = {};
    std::map<rp_channel_t,void*> m_buffer = {};

    auto clear() -> void{
        m_bits.clear();
        m_samplesCount.clear();
        m_buffer.clear();
    };
};

}

#endif