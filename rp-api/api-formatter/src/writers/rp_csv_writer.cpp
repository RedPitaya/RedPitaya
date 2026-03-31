/**
 * $Id$
 *
 * @brief Red Pitaya data formatter.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "rp_csv_writer.h"

using namespace rp_formatter_api;
using namespace std;

struct CCSVWriter::Impl {
    uint32_t m_OSCRate;
    string m_devider = ",";
    bool m_initHeader = true;
    auto write(SBufferPack* _pack, std::iostream* _memory) -> bool;
};

CCSVWriter::CCSVWriter(uint32_t _oscRate) {
    m_pimpl = new Impl();
    m_pimpl->m_OSCRate = _oscRate;
}

CCSVWriter::~CCSVWriter() {
    delete m_pimpl;
}

auto CCSVWriter::resetHeaderInit() -> void {
    m_pimpl->m_initHeader = true;
}

auto CCSVWriter::Impl::write(SBufferPack* _pack, std::iostream* _memory) -> bool {
    if (m_initHeader) {
        std::string s = "";
        bool first = true;

        rp_channel_t special_fields[] = {RP_F_INDEX, RP_F_TIME};

        for (auto field : special_fields) {
            if (_pack->m_buffer.count(field)) {
                auto name = _pack->m_name[field];
                std::string name_ch = (name == "" ? SBufferPack::getChannelName(field) : name);
                s += (first ? "" : m_devider) + name_ch;
                first = false;
            }
        }
        for (int i = RP_F_CH1; i <= RP_F_CH10; ++i) {
            rp_channel_t ch = static_cast<rp_channel_t>(i);
            if (_pack->m_buffer.count(ch)) {
                auto name = _pack->m_name[ch];
                std::string ch_name = (name == "" ? SBufferPack::getChannelName((rp_channel_t)i) : name);
                s += (first ? "" : m_devider) + ch_name;
                first = false;
            }
        }

        s += "\r\n";
        _memory->write(s.c_str(), s.length());
    }
    size_t max_samples = 0;
    rp_channel_t all_fields[] = {RP_F_INDEX, RP_F_TIME, RP_F_CH1, RP_F_CH2, RP_F_CH3, RP_F_CH4, RP_F_CH5, RP_F_CH6, RP_F_CH7, RP_F_CH8, RP_F_CH9, RP_F_CH10};

    for (auto ch : all_fields) {
        if (_pack->m_buffer.count(ch)) {
            max_samples = std::max(max_samples, (size_t)_pack->m_samplesCount[ch]);
        }
    }

    for (size_t i = 0; i < max_samples; i++) {
        std::string s = "";
        bool first_in_row = true;

        for (auto ch : all_fields) {
            if (_pack->m_buffer.count(ch)) {
                if (!first_in_row)
                    s += m_devider;
                first_in_row = false;

                if (i < _pack->m_samplesCount[ch]) {
                    void* ptr = _pack->m_buffer[ch];
                    auto bit = _pack->m_bits[ch];

                    switch (bit) {
                        case RP_F_ui8_Bit:
                            s += std::to_string(((uint8_t*)ptr)[i]);
                            break;
                        case RP_F_ui16_Bit:
                            s += std::to_string(((uint16_t*)ptr)[i]);
                            break;
                        case RP_F_ui32_Bit:
                            s += std::to_string(((uint32_t*)ptr)[i]);
                            break;
                        case RP_F_i32_Bit:
                            s += std::to_string(((int32_t*)ptr)[i]);
                            break;
                        case RP_F_ui64_Bit:
                            s += std::to_string(((uint64_t*)ptr)[i]);
                            break;
                        case RP_F_i64_Bit:
                            s += std::to_string(((int64_t*)ptr)[i]);
                            break;
                        case RP_F_f32_Bit:
                            s += std::to_string(((float*)ptr)[i]);
                            break;
                        case RP_F_d64_Bit:
                            s += std::to_string(((double*)ptr)[i]);
                            break;
                        default:
                            s += "0";
                            break;
                    }
                } else {
                    s += "0";
                }
            }
        }

        if (i < max_samples - 1) {
            s += "\r\n";
        }

        _memory->write(s.c_str(), s.length());
    }

    return true;
}

auto CCSVWriter::writeToStream(SBufferPack* _pack, std::iostream* _memory) -> bool {
    return m_pimpl->write(_pack, _memory);
}
