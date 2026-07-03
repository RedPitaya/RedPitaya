#ifndef __RP_COMMON_H__
#define __RP_COMMON_H__

#include <map>
#include <string>
#include "rp_formatter.h"

namespace rp_formatter_api {

typedef enum { RP_F_ui8_Bit, RP_F_ui16_Bit, RP_F_ui32_Bit, RP_F_i32_Bit, RP_F_ui64_Bit, RP_F_i64_Bit, RP_F_f32_Bit, RP_F_d64_Bit } rp_bits_t;

struct SBufferPack {
    std::map<rp_channel_t, rp_bits_t> m_bits = {};
    std::map<rp_channel_t, size_t> m_samplesCount = {};
    std::map<rp_channel_t, void*> m_buffer = {};
    std::map<rp_channel_t, std::string> m_name = {};

    auto clear() -> void {
        m_bits.clear();
        m_samplesCount.clear();
        m_buffer.clear();
        m_name.clear();
    };

    static auto getBitsCount(rp_bits_t type) -> uint8_t {
        switch (type) {
            case RP_F_ui8_Bit:
                return 8;
            case RP_F_ui16_Bit:
                return 16;
            case RP_F_ui32_Bit:
            case RP_F_i32_Bit:
            case RP_F_f32_Bit:
                return 32;
            case RP_F_ui64_Bit:
            case RP_F_i64_Bit:
            case RP_F_d64_Bit:
                return 64;
            default:
                return 0;  // Unknown type
        }
    }

    static auto getTypeName(rp_bits_t type) -> const char* {
        switch (type) {
            case RP_F_ui8_Bit:
                return "uint8";
            case RP_F_ui16_Bit:
                return "uint16";
            case RP_F_ui32_Bit:
                return "uint32";
            case RP_F_i32_Bit:
                return "int32";
            case RP_F_f32_Bit:
                return "float32";
            case RP_F_ui64_Bit:
                return "uint64";
            case RP_F_i64_Bit:
                return "int64";
            case RP_F_d64_Bit:
                return "double64";
            default:
                return "unknown";
        }
    }

    static auto getWavSupport(rp_bits_t type) -> uint8_t {
        switch (type) {
            case RP_F_ui32_Bit:
            case RP_F_i32_Bit:
            case RP_F_ui64_Bit:
            case RP_F_i64_Bit:
                return false;
            default:
                return true;
        }
    }

    static const char* getChannelName(rp_channel_t channel) {
        static const char* names[] = {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8", "CH9", "CH10", "TIME", "INDEX"};

        if (channel >= 0 && channel <= RP_F_INDEX) {
            return names[channel];
        }
        return "UNKNOWN";
    }
};

}  // namespace rp_formatter_api

#endif