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

#include "rp_wav_writer.h"

using namespace rp_formatter_api;


struct CWaveWriter::Impl {
    bool m_headerInit;
    uint32_t m_numChannels;
    rp_bits_t m_bitDepth;
    uint32_t m_samplesPerChannel;
    uint32_t m_OSCRate;
    rp_endianness_t m_endianness;
    std::fstream *m_file = NULL;

    auto buildHeader(std::iostream *memory) -> void;
    auto write(SBufferPack *_pack, std::iostream *_memory) -> bool;
    auto updateSize(std::iostream *_memory,size_t _size) -> void;
};

auto addStringToFileData (std::iostream *memory, std::string s) -> void {
    memory->write(s.data(),s.size());
}

auto addInt32ToFileData (std::iostream *memory, int32_t i, rp_endianness_t _endianeess) -> void {
    char bytes[4];
    if (_endianeess == RP_F_LittleEndian){
        bytes[3] = (i >> 24) & 0xFF;
        bytes[2] = (i >> 16) & 0xFF;
        bytes[1] = (i >> 8) & 0xFF;
        bytes[0] = i & 0xFF;
    }
    else{
        bytes[0] = (i >> 24) & 0xFF;
        bytes[1] = (i >> 16) & 0xFF;
        bytes[2] = (i >> 8) & 0xFF;
        bytes[3] = i & 0xFF;
    }
    memory->write(bytes,4);
}

auto addInt16ToFileData (std::iostream *memory, int16_t i, rp_endianness_t _endianeess) -> void {
    char bytes[2];
    if (_endianeess == RP_F_LittleEndian){
        bytes[1] = (i >> 8) & 0xFF;
        bytes[0] = i & 0xFF;
    }
    else{
        bytes[0] = (i >> 8) & 0xFF;
        bytes[1] = i & 0xFF;
    }
    memory->write(bytes,2);
}

CWaveWriter::CWaveWriter(uint32_t _oscRate){
    m_pimpl = new Impl();
    resetHeaderInit();
    m_pimpl->m_endianness = RP_F_LittleEndian;
    m_pimpl->m_OSCRate = _oscRate;
    m_pimpl->m_numChannels = 0;
    m_pimpl->m_file = NULL;
}

CWaveWriter::~CWaveWriter(){
    delete m_pimpl;
}

auto CWaveWriter::setEndiannes(rp_endianness_t _endiannes) -> void{
    m_pimpl->m_endianness = _endiannes;
}

auto CWaveWriter::resetHeaderInit() -> void{
    m_pimpl->m_headerInit = true;
}

auto CWaveWriter::Impl::buildHeader(std::iostream *memory) -> void {
    int sampleRate = 44100;
    int32_t dataChunkSize = m_samplesPerChannel * m_numChannels * (m_bitDepth / 8);
    int16_t data_format = m_bitDepth == 32 ? 0x0003: 0x0001;
    addStringToFileData(memory,"RIFF");

    int32_t fileSizeInBytes = 4 + 24 + 8 + dataChunkSize;
    addInt32ToFileData (memory, fileSizeInBytes, m_endianness);
    addStringToFileData(memory,"WAVE");

    // -----------------------------------------------------------
    // FORMAT CHUNK
    addStringToFileData(memory,"fmt ");
    addInt32ToFileData (memory, 16, m_endianness); // format chunk size (16 for PCM)
    addInt16ToFileData (memory, data_format, m_endianness); // audio format = 1
    addInt16ToFileData (memory, (int16_t)m_numChannels, m_endianness); // num channels
    addInt32ToFileData (memory, (int32_t)m_OSCRate, m_endianness); // sample rate

    int32_t numBytesPerSecond = (int32_t) ((m_numChannels * sampleRate * m_bitDepth) / 8);
    addInt32ToFileData (memory, numBytesPerSecond, m_endianness);

    int16_t numBytesPerBlock = m_numChannels * (m_bitDepth / 8);
    addInt16ToFileData (memory, numBytesPerBlock, m_endianness);

    addInt16ToFileData (memory, (int16_t)m_bitDepth, m_endianness);

    // -----------------------------------------------------------
    memory->write("data",4);
    addInt32ToFileData (memory, dataChunkSize, m_endianness);
}


auto CWaveWriter::Impl::write(SBufferPack *_pack, std::iostream *_memory) -> bool {
    // IF resolution = 32bit this FLOAT type data
    // Init variables

    m_numChannels = 0;
    size_t maxSamples = 0;
    rp_bits_t maxBitBySample = RP_F_8_Bit;
    std::vector<void*> channels;
    std::vector<rp_bits_t>  channelsBits;
    std::vector<size_t>   channelsSamples;

    for (auto ch = RP_F_CH1; ch <= RP_F_CH10; ch = rp_channel_t(ch + 1)){
        if (_pack->m_buffer.count(ch)) {
            m_numChannels++;
            auto sCount = _pack->m_samplesCount[ch];
            auto bits = _pack->m_bits[ch];
            auto buffer = _pack->m_buffer[ch];

            maxSamples = maxSamples < sCount ? sCount: maxSamples;
            maxBitBySample = maxBitBySample < bits ? bits : maxBitBySample;
            channels.push_back(buffer);
            channelsBits.push_back(bits);
            channelsSamples.push_back(sCount);
        }
    }

    if (maxBitBySample > RP_F_32_Bit){
        maxBitBySample = RP_F_32_Bit;
    }

    m_samplesPerChannel = maxSamples;
    m_bitDepth = maxBitBySample;


    // std::stringstream *memory = new std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    if (m_headerInit)
    {
        buildHeader(_memory);
        m_headerInit = false;
    }

    auto get16Bit = [&](uint8_t ch,size_t pos) -> uint16_t{
        if (channelsBits[ch] == RP_F_8_Bit){
            if (channelsSamples[ch] > pos){
                auto b = (uint8_t*)channels[ch];
                return ((uint16_t)b[pos]) << 8;
            }
        }

        if (channelsBits[ch] == RP_F_16_Bit){
            if (channelsSamples[ch] > pos){
                auto b = (uint16_t*)channels[ch];
                return b[pos];
            }
        }
        return 0;
    };

    auto get32Bit = [&](uint8_t ch,size_t pos) -> float{
        if (channelsBits[ch] == RP_F_8_Bit){
            if (channelsSamples[ch] > pos){
                auto b = (int8_t*)channels[ch]; // keep sign
                return ((float)b[pos]) / (float)0x7F;
            }
        }

        if (channelsBits[ch] == RP_F_16_Bit){
            if (channelsSamples[ch] > pos){
                auto b = (int16_t*)channels[ch];  // keep sign
                return ((float)b[pos]) / (float)0x7FFF;
            }
        }

        if (channelsBits[ch] == RP_F_32_Bit){
            if (channelsSamples[ch] > pos){
                auto b = (float*)channels[ch];
                return b[pos];
            }
        }

        if (channelsBits[ch] == RP_F_64_Bit){
            if (channelsSamples[ch] > pos){
                auto b = (double*)channels[ch];
                return b[pos];
            }
        }
        return 0;
    };

    size_t buffLen = m_numChannels * maxSamples * (maxBitBySample / 8);
    try{
        uint8_t* cross_buff = new uint8_t[buffLen];

        for(size_t i = 0; i < maxSamples; i++){
            for(uint8_t ch = 0; ch < m_numChannels; ch++){
                if (m_bitDepth == 8){
                    if (channelsSamples[ch] > i){
                        cross_buff[i * m_numChannels + ch] = ((uint8_t*)channels[ch])[i];
                    }else{
                        cross_buff[i * m_numChannels + ch] = 0;
                    }
                }

                if (m_bitDepth == 16){
                    auto cross_buff16 = (uint16_t*)cross_buff;
                    if (channelsSamples[ch] > i){
                        cross_buff16[i * m_numChannels + ch] = get16Bit(ch,i);
                    }else{
                        cross_buff16[i * m_numChannels + ch] = 0;
                    }
                }

                if (m_bitDepth == 32){
                    auto cross_buff32 = (float*)cross_buff;
                    if (channelsSamples[ch] > i){
                        cross_buff32[i * m_numChannels + ch] = get32Bit(ch,i);
                    }else{
                        cross_buff32[i * m_numChannels + ch] = 0;
                    }
                }
            }
        }
        _memory->write((const char*)cross_buff, buffLen);
        _memory->flush();
        updateSize(_memory,buffLen);
        delete [] cross_buff;
        return true;
    }catch(std::exception &e){
        fprintf(stderr,"[ERROR] CWaveWriter::write %s\n",e.what());
    }
    return false;
}

auto CWaveWriter::writeToStream(SBufferPack *_pack, std::iostream *_memory) -> bool {
    return m_pimpl->write(_pack,_memory);
}

auto CWaveWriter::Impl::updateSize(std::iostream *_memory,size_t _size) -> void {
    int offset1 = 4;
    int offset2 = 40;

    auto cur_p = _memory->tellp();
    auto cur_g = _memory->tellg();

    int32_t size1 = 0;
    int32_t size2 = 0;
    _memory->seekg(offset1, _memory->beg);
    _memory->read ((char*)&size1, sizeof(size1));

    size1 += _size;
    _memory->seekp(offset1, _memory->beg);
    _memory->write((char*)&size1, sizeof(size1));
    _memory->seekg(offset2, _memory->beg);
    _memory->read ((char*)&size2, sizeof(size2));
    size2 += _size;
    _memory->seekp(offset2, _memory->beg);
    _memory->write((char*)&size2, sizeof(size2));

    _memory->seekp(cur_p);
    _memory->seekg(cur_g);
}


