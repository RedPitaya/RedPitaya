#ifndef WAV_LIB_WAVWRITER_H
#define WAV_LIB_WAVWRITER_H

#include <iostream>
#include "writer_lib/file_helper.h"

class CWaveWriter {
    enum class Endianness { LittleEndian, BigEndian };

   public:
    CWaveWriter();
    auto resetHeaderInit() -> void;
    // The interleaved samples of the buffers, after the header for the first chunk; nullptr when
    // there is no memory for them
    auto BuildWAVChunk(std::map<DataLib::EDataBuffersPackChannel, SBuffPass> new_buffs) -> FileChunk::Ptr;

   private:
    auto addInt32ToFileData(std::vector<uint8_t>& memory, int32_t i) -> void;
    auto addInt16ToFileData(std::vector<uint8_t>& memory, int16_t i) -> void;
    auto addStringToFileData(std::vector<uint8_t>& memory, std::string s) -> void;
    auto BuildHeader(std::vector<uint8_t>& memory) -> void;

    bool m_headerInit;
    uint32_t m_numChannels;
    uint8_t m_bitDepth;
    uint32_t m_samplesPerChannel;
    uint32_t m_OSCRate;
    CWaveWriter::Endianness m_endianness;
};

#endif
