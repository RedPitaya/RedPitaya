#pragma once

#include <fstream>
#include <iostream>

class CWaveWriter
{
    enum class Endianness
    {
        LittleEndian,
        BigEndian
    };

public:
    CWaveWriter();
    auto resetHeaderInit() -> void;
    auto BuildWAVStream(uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2,unsigned short resolution) -> std::iostream *;

private:
    auto addInt32ToFileData (std::stringstream *memory, int32_t i) -> void;
    auto addInt16ToFileData (std::stringstream *memory, int16_t i) -> void;
    auto addStringToFileData (std::stringstream *memory, std::string s) -> void;
    auto BuildHeader(std::stringstream *memory) -> void;

    bool m_headerInit;
    int  m_numChannels;
    int  m_bitDepth;
    int  m_samplesPerChannel;
    CWaveWriter::Endianness m_endianness;
};
