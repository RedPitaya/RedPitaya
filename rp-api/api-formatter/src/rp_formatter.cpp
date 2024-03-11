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

#include "rp_formatter.h"
#include "writers/rp_tdms_writer.h"
#include "writers/rp_wav_writer.h"
#include "writers/rp_csv_writer.h"

#include "writers/common.h"


using namespace rp_formatter_api;

struct CFormatter::Impl {
    rp_mode_t m_mode;
    uint32_t m_oscRate;
    SBufferPack m_pack;
    std::fstream *m_file = NULL;  
    CWaveWriter *m_wave = NULL;
    CTDMSWriter *m_tdms = NULL;
    CCSVWriter  *m_csv = NULL;

    ~Impl(){
        delete m_wave;
        delete m_tdms;
        delete m_csv;
    };
};

CFormatter::CFormatter(rp_mode_t _mode,uint32_t _oscRate) {
    m_pimpl = new Impl();
    m_pimpl->m_oscRate = _oscRate;
    m_pimpl->m_wave = _mode == RP_F_WAV ? new CWaveWriter(_oscRate) : NULL;
    m_pimpl->m_tdms = _mode == RP_F_TDMS ? new CTDMSWriter(_oscRate) : NULL;
    m_pimpl->m_csv = _mode == RP_F_CSV ? new CCSVWriter(_oscRate) : NULL;
}

CFormatter::~CFormatter(){
    closeFile();
    delete m_pimpl;
}

auto CFormatter::setEndiannes(rp_endianness_t _endiannes) -> bool {
    if (m_pimpl->m_wave) {
        m_pimpl->m_wave->setEndiannes(_endiannes);
        return true;
    }
    return false;
}

auto CFormatter::resetWriter() -> void {
    if (m_pimpl->m_wave) {
        m_pimpl->m_wave->resetHeaderInit();
    }
    if (m_pimpl->m_csv) {
        m_pimpl->m_csv->resetHeaderInit();
    }
}

auto CFormatter::clearBuffer() -> void {
    m_pimpl->m_pack.clear();
}

auto CFormatter::setChannelData(rp_channel_t _channel,void* _buffer,size_t _samplesCount,rp_bits_t _bits,std::string &_name) -> void {
    m_pimpl->m_pack.m_bits[_channel] = _bits;
    m_pimpl->m_pack.m_samplesCount[_channel] = _samplesCount;
    m_pimpl->m_pack.m_buffer[_channel] = _buffer;
    m_pimpl->m_pack.m_name[_channel] = _name;
}

auto CFormatter::setChannel(rp_channel_t _channel,uint8_t* _buffer,size_t _samplesCount,std::string _name) -> void {
    setChannelData(_channel,_buffer,_samplesCount,RP_F_8_Bit,_name);
}

auto CFormatter::setChannel(rp_channel_t _channel,uint16_t* _buffer,size_t _samplesCount,std::string _name) -> void {
    setChannelData(_channel,_buffer,_samplesCount,RP_F_16_Bit,_name);
}

auto CFormatter::setChannel(rp_channel_t _channel,float* _buffer,size_t _samplesCount,std::string _name) -> void {
    setChannelData(_channel,_buffer,_samplesCount,RP_F_32_Bit,_name);
}

auto CFormatter::setChannel(rp_channel_t _channel,double* _buffer,size_t _samplesCount,std::string _name) -> void {
    setChannelData(_channel,_buffer,_samplesCount,RP_F_64_Bit,_name);
}

auto CFormatter::openFile(std::string _path) -> bool {
    if (m_pimpl->m_file){
        return false;
    }
    m_pimpl->m_file = new std::fstream(_path,std::ios::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
    if (m_pimpl->m_file->is_open()){
        return true;
    }
    delete m_pimpl->m_file;
    m_pimpl->m_file = NULL;
    return false;
}

auto CFormatter::closeFile() -> bool {
    if (m_pimpl->m_file){
        if (m_pimpl->m_file->is_open()){
            m_pimpl->m_file->flush();
            m_pimpl->m_file->close();
        }
        delete m_pimpl->m_file;
        m_pimpl->m_file = NULL;
        return true;
    }
    return false;
}

auto CFormatter::isOpenFile() -> bool{
    if (m_pimpl->m_file){
        if (m_pimpl->m_file->is_open()){
            return true;
        }
    }
    return false;
}

auto CFormatter::writeToFile() -> bool {
    if (!isOpenFile()){
        return false;
    }
    if (m_pimpl->m_wave){
        return m_pimpl->m_wave->writeToStream(&m_pimpl->m_pack,m_pimpl->m_file);
    }
    if (m_pimpl->m_tdms){
        return m_pimpl->m_tdms->writeToStream(&m_pimpl->m_pack,m_pimpl->m_file);
    }
    if (m_pimpl->m_csv){
        return m_pimpl->m_csv->writeToStream(&m_pimpl->m_pack,m_pimpl->m_file);
    }
    return false;
}

auto CFormatter::writeToStream(std::iostream *_memory) -> bool {
    if (m_pimpl->m_wave){
        return m_pimpl->m_wave->writeToStream(&m_pimpl->m_pack,_memory);
    }
    if (m_pimpl->m_tdms){
        return m_pimpl->m_tdms->writeToStream(&m_pimpl->m_pack,_memory);
    }
    if (m_pimpl->m_csv){
        return m_pimpl->m_csv->writeToStream(&m_pimpl->m_pack,_memory);
    }
    return false;
}


