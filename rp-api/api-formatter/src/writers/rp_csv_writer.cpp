/**
 * $Id$
 *
 * @brief Red Pitaya data formatter.
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#include <cassert>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <ctime>
#include <iostream>
#include <chrono>

#include "rp_csv_writer.h"

using namespace rp_formatter_api;
using namespace std;

struct CCSVWriter::Impl {
    uint32_t m_OSCRate;
    string m_devider = ",";
    bool m_initHeader = true;
    auto write(SBufferPack *_pack, std::iostream *_memory) -> bool;
};


CCSVWriter::CCSVWriter(uint32_t _oscRate){
    m_pimpl = new Impl();
    m_pimpl->m_OSCRate = _oscRate;    
}

CCSVWriter::~CCSVWriter(){
    delete m_pimpl;
}

auto CCSVWriter::resetHeaderInit() -> void{
    m_pimpl->m_initHeader = true;
}


auto CCSVWriter::Impl::write(SBufferPack *_pack, std::iostream *_memory) -> bool {
    if (m_initHeader){
        std::string s = "Index";
        for (auto ch = RP_F_CH1; ch <= RP_F_CH10; ch = rp_channel_t(ch + 1)){
            if (_pack->m_buffer.count(ch)) {
                auto name = _pack->m_name[ch];
                string ch_name = (name == "" ? (std::string("Ch ") + std::to_string(ch + 1)) : name);
                s += m_devider + ch_name;
            }
        }
        s += "\r\n";
        _memory->write(s.c_str(),s.length());
    }

    size_t max_samples = 0;
    for (auto ch = RP_F_CH1; ch <= RP_F_CH10; ch = rp_channel_t(ch + 1)){
        if (_pack->m_buffer.count(ch)) {
            max_samples = _pack->m_samplesCount[ch] > max_samples ? _pack->m_samplesCount[ch] : max_samples;
        }
    }

    for(size_t i = 0; i < max_samples; i++){
        std::string s = to_string(i + 1);
        for (auto ch = RP_F_CH1; ch <= RP_F_CH10; ch = rp_channel_t(ch + 1)){
            if (_pack->m_buffer.count(ch)) {
                s += m_devider;
                auto bit = _pack->m_bits[ch];
                auto sCount = _pack->m_samplesCount[ch];
                if (sCount > i){
                    if (bit == RP_F_8_Bit){
                        auto buff = (int8_t*)_pack->m_buffer[ch];
                        s += to_string(buff[i]);
                    }
                    if (bit == RP_F_16_Bit){
                        auto buff = (int16_t*)_pack->m_buffer[ch];
                        s += to_string(buff[i]);
                    }
                    if (bit == RP_F_32_Bit){
                        auto buff = (float*)_pack->m_buffer[ch];
                        s += to_string(buff[i]);
                    }
                    if (bit == RP_F_64_Bit){
                        auto buff = (double*)_pack->m_buffer[ch];
                        s += to_string(buff[i]);
                    }
                }else{
                    s += "0";
                }
            }
        }
        s += (i != (max_samples-1)) ? "\r\n" : "";
        _memory->write(s.c_str(),s.length());
    }

    return true;
}


auto CCSVWriter::writeToStream(SBufferPack *_pack, std::iostream *_memory) -> bool {
    return m_pimpl->write(_pack, _memory);
}


