#ifndef _WIN32
#include <sys/statvfs.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <cstring>
#include <limits>
#include <sstream>

#include "file_helper.h"
#include "data_lib/thread_cout.h"
#include "tdms_lib/file.h"
#include "w_binary.h"

static constexpr uint8_t g_endOfSegment[12] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


auto getTotalSystemMemory() -> uint64_t{
#ifndef _WIN32
    uint64_t pages = sysconf(_SC_PHYS_PAGES);
    uint64_t page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#else
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#endif
}

auto availableSpace(std::string dst,  uint64_t* availableSize) -> int {
#ifdef _WIN32
    *availableSize = UINT64_MAX;
    return 0;
#else
    int result = -1;
    try {
        struct statvfs devData;
        memset(&devData, 0, sizeof (struct statvfs));
        if ((statvfs(dst.c_str(), &devData)) >= 0) {
            if (availableSize != NULL) {
                //I don't know if it's right, but I'll set availableSize only if the available size doesn't pass the ulong limit.
                if (devData.f_bavail  > (std::numeric_limits<uint64_t>::max() / devData.f_bsize)) {
                    *availableSize = std::numeric_limits<uint64_t>::max();
                } else {
                    *availableSize = (uint64_t)devData.f_bavail * (uint64_t)devData.f_bsize;
                }
            }
            result = 0;
        }
    }
    catch (std::exception& e)
    {
        aprintf(stderr,"Error in AvailableSpace(): %s\n",e.what());
    }
    return result;
#endif
}

auto getFreeSpaceDisk(std::string _filePath) -> uint64_t{
    uint64_t m_freeSize = 0;
    if (availableSpace(_filePath, &m_freeSize) == 0){
        if (m_freeSize < USING_FREE_SPACE){
            m_freeSize = 0;
        }else{
            m_freeSize = m_freeSize - USING_FREE_SPACE;
        }
    }else{
        m_freeSize = 0;
    }
    return  m_freeSize;
}

auto dirNameOf(const std::string& fname) -> std::string{
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos)
           ? "."
           : fname.substr(0, pos);
}


auto buildTDMSStream(std::map<DataLib::EDataBuffersPackChannel,SBuffPass> new_buffs) -> std::iostream *{
    TDMS::File outFile;
    TDMS::WriterSegment segment;
    vector<shared_ptr<TDMS::Metadata>> data;
//    std::time_t tim_sec = std::time(0);
//    std::time_t local = std::mktime(std::localtime(&tim_sec));
//    std::time_t gmt = std::mktime(std::gmtime(&tim_sec));
//    auto timezone = static_cast<long> (local - gmt);

    auto root = segment.GenerateRoot();
    root->TableOfContents.HasMetaData = true;
    root->TableOfContents.HasRawData = true;
    data.push_back(root);
    auto group = segment.GenerateGroup("Group");
    data.push_back(group);

//    auto *time = TDMS::DataType::GetRawTimeValue(tim_sec + timezone);
//    TDMS::DataType dataprop;
//    dataprop.InitDataType(TDMS::DataType::TimeStamp,time);
//    segment.AddProperties(root,"time_stamp_now",dataprop);



    if (new_buffs.find(DataLib::CH1) != new_buffs.end())
    {
        auto settings = new_buffs.at(DataLib::CH1);
        if (settings.bufferLen){
            auto data_type = TDMS::TDMSType::Integer8;
            if (settings.bitsBySample == 16) data_type = TDMS::TDMSType::Integer16;
            if (settings.bitsBySample == 32) data_type = TDMS::TDMSType::SingleFloat;
            auto sampelsCount = settings.samplesCount;
            auto buffer = settings.buffer;
            auto channel = segment.GenerateChannel("Group", "ch1");
            data.push_back(channel);
            segment.AddRaw(channel, data_type, sampelsCount , buffer);
        }
    }

    if (new_buffs.find(DataLib::CH2) != new_buffs.end())
    {
        auto settings = new_buffs.at(DataLib::CH2);
        if (settings.bufferLen){
            auto data_type = TDMS::TDMSType::Integer8;
            if (settings.bitsBySample == 16) data_type = TDMS::TDMSType::Integer16;
            if (settings.bitsBySample == 32) data_type = TDMS::TDMSType::SingleFloat;
            auto sampelsCount = settings.samplesCount;
            auto buffer = settings.buffer;
            auto channel = segment.GenerateChannel("Group", "ch2");
            data.push_back(channel);
            segment.AddRaw(channel, data_type, sampelsCount , buffer);
        }
    }

    if (new_buffs.find(DataLib::CH3) != new_buffs.end())
    {
        auto settings = new_buffs.at(DataLib::CH3);
        if (settings.bufferLen){
            auto data_type = TDMS::TDMSType::Integer8;
            if (settings.bitsBySample == 16) data_type = TDMS::TDMSType::Integer16;
            if (settings.bitsBySample == 32) data_type = TDMS::TDMSType::SingleFloat;
            auto sampelsCount = settings.samplesCount;
            auto buffer = settings.buffer;
            auto channel = segment.GenerateChannel("Group", "ch3");
            data.push_back(channel);
            segment.AddRaw(channel, data_type, sampelsCount , buffer);
        }
    }

    if (new_buffs.find(DataLib::CH4) != new_buffs.end())
    {
        auto settings = new_buffs.at(DataLib::CH4);
        if (settings.bufferLen){
            auto data_type = TDMS::TDMSType::Integer8;
            if (settings.bitsBySample == 16) data_type = TDMS::TDMSType::Integer16;
            if (settings.bitsBySample == 32) data_type = TDMS::TDMSType::SingleFloat;
            auto sampelsCount = settings.samplesCount;
            auto buffer = settings.buffer;
            auto channel = segment.GenerateChannel("Group", "ch4");
            data.push_back(channel);
            segment.AddRaw(channel, data_type, sampelsCount , buffer);
        }
    }

    segment.LoadMetadata(data);
    stringstream *memory = new stringstream(ios_base::in | ios_base::out | ios_base::binary);
    outFile.WriteMemory(*memory,segment);
    return memory;
}

auto buildBINStream(DataLib::CDataBuffersPack::Ptr buff_pack,std::map<DataLib::EDataBuffersPackChannel,uint32_t> _samples) -> std::iostream *{
    stringstream *memory = new stringstream(ios_base::in | ios_base::out | ios_base::binary);
    CBinInfo::BinHeader header;
    DataLib::CDataBuffer::Ptr ch[4] = {NULL,NULL,NULL,NULL};
    ch[0] = buff_pack->getBuffer(DataLib::CH1);
    ch[1] = buff_pack->getBuffer(DataLib::CH2);
    ch[2] = buff_pack->getBuffer(DataLib::CH3);
    ch[3] = buff_pack->getBuffer(DataLib::CH4);
    uint32_t ch_samp[4] = {0,0,0,0};
    uint32_t ch_size[4] = {0,0,0,0};
    memset(header.dataFormatSize,0,sizeof(uint8_t) * 4);
    memset(header.sizeCh,0,sizeof(uint32_t) * 4);

    for(int i = 0; i < 4; i++){
        ch_samp[i] = ch[i]->getSamplesCount() < _samples[DataLib::CH1] ? ch[i]->getSamplesCount() : _samples[DataLib::CH1];
        auto bytes = ch[i]->getBitBySample() / 8;
        ch_size[i] = ch_samp[i] * bytes > ch[i]->getBufferLenght() ? ch[i]->getBufferLenght() : ch_samp[i] * bytes;
        header.dataFormatSize[i] = ch[i]->getBitBySample() / 8;
        header.sizeCh[i] = ch_size[i];
        header.sampleCh[i] = ch_samp[i];
        header.lostCount[i] = ch[i]->getLostSamples(DataLib::FPGA) + ch[i]->getLostSamples(DataLib::RP_INTERNAL_BUFFER);
    }

    header.sigmentLength = header.sizeCh[0] + header.sizeCh[1] + header.sizeCh[2] + header.sizeCh[3];
    //Write header
    memory->write((const char*)&header,sizeof(header));
    for(int i = 0; i < 4; i++){
        if (ch[i] && ch_size[i]) memory->write((const char*)ch[i]->getBuffer().get(), ch_size[i]);
    }
    //Write end segment
    memory->write((const char*)g_endOfSegment,12);
    return memory;
}

auto readCSV(std::iostream *buffer, int64_t *_position,int *_channels,uint64_t *samplePos,bool skipData) -> std::iostream*{
    uint32_t endSeg[] = { 0, 0 ,0};
    stringstream *memory = nullptr;
    buffer->seekg(*_position, std::ios::beg);
    CBinInfo::BinHeader header;
    buffer->read((char*)&header, sizeof(header));
    buffer->seekg(*_position + sizeof(CBinInfo::BinHeader) + header.sigmentLength, std::ios::beg);
    buffer->read((char*)endSeg , 12);
    if (endSeg[0] == 0xFFFFFFFF && endSeg[1] == 0xFFFFFFFF && endSeg[2] == 0xFFFFFFFF){
        if (!skipData){
            auto size_ch1 = header.sizeCh[0];
            auto size_ch2 = header.sizeCh[1];
            auto size_ch3 = header.sizeCh[2];
            auto size_ch4 = header.sizeCh[3];

            auto sample_ch1 = header.sampleCh[0];
            auto sample_ch2 = header.sampleCh[1];
            auto sample_ch3 = header.sampleCh[2];
            auto sample_ch4 = header.sampleCh[3];

            auto lost_ch1 = header.lostCount[0];
            auto lost_ch2 = header.lostCount[1];
            auto lost_ch3 = header.lostCount[2];
            auto lost_ch4 = header.lostCount[3];

            if (*_channels == 0) {
                *_channels = (size_ch1 > 0 ? 0x1 : 0) | (size_ch2 > 0 ? 0x2 :0) | (size_ch3 > 0 ? 0x4 : 0) | (size_ch4 > 0 ? 0x8 : 0);
            }
            if (size_ch1 || size_ch2 || size_ch3 || size_ch4 || lost_ch1 || lost_ch2 || lost_ch3 || lost_ch4) {
                memory = new stringstream(ios_base::in | ios_base::out);
            }
            auto resolutionCh1 = header.dataFormatSize[0] * 8;
            auto resolutionCh2 = header.dataFormatSize[1] * 8;
            auto resolutionCh3 = header.dataFormatSize[2] * 8;
            auto resolutionCh4 = header.dataFormatSize[3] * 8;

            char *buffer_ch1 = nullptr;
            char *buffer_ch2 = nullptr;
            char *buffer_ch3 = nullptr;
            char *buffer_ch4 = nullptr;

            buffer->seekg(*_position + sizeof(CBinInfo::BinHeader), std::ios::beg);
            if (size_ch1 > 0) {
                buffer_ch1 = new char[size_ch1];
                buffer->read(buffer_ch1,size_ch1);
            }

            if (size_ch2 > 0) {
                buffer_ch2 = new char[size_ch2];
                buffer->read(buffer_ch2,size_ch2);
            }

            if (size_ch3 > 0) {
                buffer_ch3 = new char[size_ch3];
                buffer->read(buffer_ch3,size_ch3);
            }

            if (size_ch4 > 0) {
                buffer_ch4 = new char[size_ch4];
                buffer->read(buffer_ch4,size_ch4);
            }

            auto max_size = sample_ch1 + lost_ch1;
            max_size = max_size < (sample_ch2  + lost_ch2) ? sample_ch2 + lost_ch2 : max_size;
            max_size = max_size < (sample_ch3  + lost_ch3) ? sample_ch3 + lost_ch3 : max_size;
            max_size = max_size < (sample_ch4  + lost_ch4) ? sample_ch4 + lost_ch4 : max_size;

            auto print = [&](char *b,size_t pos,uint8_t res){
                if (res == 8) {
                   *memory << (int)((int8_t*)b)[pos];
                }

                if (res == 16) {
                   *memory << ((int16_t*)b)[pos];
                }

                if (res == 32) {
                   *memory << ((float*)b)[pos];
                }
            };

            for(auto i = 0u ;i < max_size; i++){
                (*samplePos)++;
                *memory << *samplePos << ",";
                bool needPrintComma = false;
                if (i < sample_ch1){
                    needPrintComma = true;
                    print(buffer_ch1,i,resolutionCh1);
                }else{
                    if (sample_ch1 > 0 || lost_ch1 > 0){
                        needPrintComma = true;
                        *memory << "-";
                    }
                }

                if (i < sample_ch2){
                    if (needPrintComma)
                        *memory << ",";
                    needPrintComma = true;
                    print(buffer_ch2,i,resolutionCh2);
                }else{
                    if (sample_ch2 > 0 || lost_ch2 > 0){
                        if (needPrintComma)
                            *memory << ",";
                        needPrintComma = true;
                        *memory << "-";
                    }
                }

                if (i < sample_ch3){
                    if (needPrintComma)
                        *memory << ",";
                    needPrintComma = true;
                    print(buffer_ch3,i,resolutionCh3);
                }else{
                    if (sample_ch3 > 0 || lost_ch3 > 0){
                        if (needPrintComma)
                            *memory << ",";
                        needPrintComma = true;
                        *memory << "-";
                    }
                }

                if (i < sample_ch4){
                    if (needPrintComma)
                        *memory << ",";
                    print(buffer_ch4,i,resolutionCh4);
                }else{
                    if (sample_ch4 > 0 || lost_ch4 > 0){
                        if (needPrintComma)
                            *memory << ",";
                        *memory << "-";
                    }
                }
                *memory << "\n";
            }

            delete[] buffer_ch1;
            delete[] buffer_ch2;
            delete[] buffer_ch3;
            delete[] buffer_ch4;
        }
        buffer->seekg(0, std::ios::end);
        auto Length = buffer->tellg();
        *_position = *_position + sizeof(CBinInfo::BinHeader) + header.sigmentLength + 12; // 12 - End segment len
        if (*_position >= Length) {
            *_position = -2;
        }
    }else{
        *_position = -1;
    }
    return memory;
}

auto readBinInfo(std::iostream *buffer) -> CBinInfo{
    int64_t position = 0;
    buffer->seekg(0, std::ios::end);
    auto Length = buffer->tellg();
    CBinInfo bi;
    while(position >= 0){
        uint32_t endSeg[] = { 0, 0 ,0};
        buffer->seekg(position, std::ios::beg);
        CBinInfo::BinHeader header;
        buffer->read((char*)&header, sizeof(CBinInfo::BinHeader));
        buffer->seekg(position + sizeof(CBinInfo::BinHeader) + header.sigmentLength, std::ios::beg);
        buffer->read((char*)endSeg , 12);
        uint64_t samplesCount = 0u;
        for(auto i = 0u; i < 4 ; i++){
            bi.dataFormatSize[i] = header.dataFormatSize[i];
            bi.size_ch[i]  += header.sizeCh[i];
            if (header.dataFormatSize[i]){
                samplesCount += header.sizeCh[i] / header.dataFormatSize[i];
                bi.samples_ch[i] = header.sizeCh[i] / header.dataFormatSize[i];
            }
            bi.lostCount[i] = header.lostCount[i];
        }

        if (bi.segSamplesCount == 0) bi.segSamplesCount = samplesCount;
        bi.segLastSamplesCount = samplesCount;
        bi.segCount++;
        if (endSeg[0] == 0xFFFFFFFF && endSeg[1] == 0xFFFFFFFF && endSeg[2] == 0xFFFFFFFF){
            position =  position + sizeof(CBinInfo::BinHeader) + header.sigmentLength + 12;
            if (position >= Length) {
                position = -2;
            }
        }else{
            position = -1;
        }
    }
    bi.lastSegState = position == -2;
    return bi;
}
