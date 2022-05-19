#ifndef WAV_LIB_WAVWRITER_H
#define WAV_LIB_WAVWRITER_H

#include <fstream>
#include <iostream>
#include "writer_lib/file_helper.h"

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
    auto BuildWAVStream(std::map<DataLib::EDataBuffersPackChannel,SBuffPass> new_buffs) -> std::iostream *;

private:
    auto addInt32ToFileData (std::stringstream *memory, int32_t i) -> void;
    auto addInt16ToFileData (std::stringstream *memory, int16_t i) -> void;
    auto addStringToFileData (std::stringstream *memory, std::string s) -> void;
    auto BuildHeader(std::stringstream *memory) -> void;

    bool m_headerInit;
    uint32_t m_numChannels;
    uint8_t  m_bitDepth;
    uint32_t  m_samplesPerChannel;
    CWaveWriter::Endianness m_endianness;
};

#endif
