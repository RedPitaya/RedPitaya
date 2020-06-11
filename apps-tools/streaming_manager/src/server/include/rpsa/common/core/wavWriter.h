#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <list>
#include <asio.hpp>
#include <fstream>
#include <iostream>

class CWaveWriter
{
enum class Endianness
    {
        LittleEndian,
        BigEndian
    };

    bool m_headerInit;
    int  m_numChannels;
    int  m_bitDepth;
    int  m_samplesPerChannel;
    CWaveWriter::Endianness m_endianness;

    
public:

    CWaveWriter();
    void resetHeaderInit();
    std::iostream *BuildWAVStream(uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2,unsigned short resolution);
private:
    void BuildHeader(std::stringstream *memory);
    void addInt32ToFileData (std::stringstream *memory, int32_t i);
    void addInt16ToFileData (std::stringstream *memory, int16_t i);
    void addStringToFileData (std::stringstream *memory, std::string s);
    
};
