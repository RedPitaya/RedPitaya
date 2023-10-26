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

#include "rp_tdms_writer.h"
#include "tdms/file.h"
#include "tdms/writer.h"

using namespace rp_formatter_api;
using namespace std;

struct CTDMSWriter::Impl {
    uint32_t m_OSCRate;
    auto write(SBufferPack *_pack, std::iostream *_memory) -> bool;
};


CTDMSWriter::CTDMSWriter(uint32_t _oscRate){
    m_pimpl = new Impl();
    m_pimpl->m_OSCRate = _oscRate;
}

CTDMSWriter::~CTDMSWriter(){
    delete m_pimpl;
}

bool isLeapYear(int year) {
    if (year % 400 == 0) {
        return true;
    }
    if (year % 100 == 0) {
        return false;
    }
    if (year % 4 == 0) {
        return true;
    }
    return false;
}

auto CTDMSWriter::Impl::write(SBufferPack *_pack, std::iostream *_memory) -> bool {
    TDMS::File outFile;
    TDMS::WriterSegment segment;
    vector<shared_ptr<TDMS::Metadata>> data;

    auto root = segment.GenerateRoot();
    root->TableOfContents.HasMetaData = true;
    root->TableOfContents.HasRawData = true;
    data.push_back(root);
    auto group = segment.GenerateGroup("Group");
    data.push_back(group);

    auto now = std::chrono::system_clock::now();
    auto currentSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
	uint64_t diffSeconds = 0;
    for(int i = 1904; i < 1970; i++){
        diffSeconds += 24 * 60 * 60 * (isLeapYear(i) ? 366 : 365);
    }
    auto totalSeconds = currentSeconds + diffSeconds;

    auto d = new uint64_t[2];
    d[0] = 0;
    d[1] = totalSeconds;

    TDMS::DataType data_prop;
    data_prop.InitDataType(TDMS::TDMSType::TimeStamp,d);
    segment.AddProperties(group,"time",data_prop);
    TDMS::DataType osc_prop;
    auto osc = new uint64_t[1];
    osc[0] = m_OSCRate;
    osc_prop.InitDataType(TDMS::TDMSType::UnsignedInteger64,osc);
    segment.AddProperties(group,"osc_rate",osc_prop);

    for (auto ch = RP_F_CH1; ch <= RP_F_CH10; ch = rp_channel_t(ch + 1)){
        if (_pack->m_buffer.count(ch)) {
            auto sCount = _pack->m_samplesCount[ch];
            auto bits = _pack->m_bits[ch];
            auto buffer = _pack->m_buffer[ch];
            string ch_name = std::string("ch") + std::to_string(ch + 1);

            auto data_type = TDMS::TDMSType::Integer8;
            if (bits == RP_F_8_Bit) data_type = TDMS::TDMSType::Integer8;
            if (bits == RP_F_16_Bit) data_type = TDMS::TDMSType::Integer16;
            if (bits == RP_F_32_Bit) data_type = TDMS::TDMSType::SingleFloat;
            if (bits == RP_F_64_Bit) data_type = TDMS::TDMSType::DoubleFloat;
       
            auto channel = segment.GenerateChannel("Group", ch_name);
            data.push_back(channel);
            segment.AddRaw(channel, data_type, sCount , reinterpret_cast<uint8_t*>(buffer));            
        }
    }

    segment.LoadMetadata(data);
    outFile.WriteMemory(*_memory,segment);
    return true;
}


auto CTDMSWriter::writeToStream(SBufferPack *_pack, std::iostream *_memory) -> bool {
    return m_pimpl->write(_pack, _memory);
}


