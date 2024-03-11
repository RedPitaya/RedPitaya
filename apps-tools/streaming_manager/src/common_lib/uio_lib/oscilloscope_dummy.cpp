#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include "oscilloscope.h"
#include <stdio.h>
#include <string.h>

using namespace uio_lib;

auto COscilloscope::create(const UioT &,uint32_t _dec_factor,bool _isMaster,uint32_t _adcMaxSpeed,bool _isADCFilterPresent,uint8_t _fpgaBits,uint8_t _maxChannels) -> COscilloscope::Ptr {
    return std::make_shared<COscilloscope>(0, nullptr, 0, nullptr, 0, 0,_dec_factor,_isMaster,_adcMaxSpeed,_isADCFilterPresent,_fpgaBits,_maxChannels);
}

COscilloscope::COscilloscope(int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t _dec_factor,bool _isMaster,uint32_t _adcMaxSpeed,bool _isADCFilterPresent,uint8_t _fpgaBits,uint8_t _maxChannels) :
    m_Fd(_fd),
    m_Regset(_regset),
    m_RegsetSize(_regsetSize),
    m_Buffer(_buffer),
    m_BufferSize(_bufferSize),
    m_BufferPhysAddr(_bufferPhysAddr),
    m_OscMap(nullptr),
    m_OscBufferNumber(0),
    m_dec_factor(_dec_factor),
    m_filterBypass(true),
    m_isMaster(_isMaster),
    m_adcMaxSpeed(_adcMaxSpeed),
    m_isADCFilterPresent(_isADCFilterPresent),
    m_fpgaBits(_fpgaBits),
    m_maxChannels(_maxChannels)
{
    for(int i = 0; i < 4; i++){
        m_OscBuffer[i] = nullptr;
    }
    
    for(auto j = 0u; j < _maxChannels; j++){
        m_OscBuffer[j] = new uint8_t[osc_buf_size];
        for(auto i = 0u ; i < osc_buf_size; i++){
            m_OscBuffer[j][i] = j % 2 ? 255 - i % 255 : i % 255;
        }
    }
}

COscilloscope::~COscilloscope(){
    for(auto j = 0u; j < m_maxChannels; j++){
        delete[] m_OscBuffer[j];
    }
}

void COscilloscope::setReg(volatile OscilloscopeMapT *){}

void COscilloscope::setFilterCalibration(uint8_t,int32_t ,int32_t , int32_t , int32_t ){}

void COscilloscope::setFilterBypass(bool){}

auto COscilloscope::prepare() -> void{}

auto COscilloscope::setCalibration(uint8_t,int32_t,float) -> void{}

auto COscilloscope::next(uint8_t *&_buffer1,uint8_t *&_buffer2,uint8_t *&_buffer3,uint8_t *&_buffer4, size_t &_size,uint32_t &_overFlow) -> bool {
    _buffer1 = m_maxChannels >= 1 ? m_OscBuffer[0] : nullptr;
    _buffer2 = m_maxChannels >= 2 ? m_OscBuffer[1] : nullptr;
    _buffer3 = m_maxChannels >= 3 ? m_OscBuffer[2] : nullptr;
    _buffer4 = m_maxChannels >= 4 ? m_OscBuffer[3] : nullptr;

    _overFlow = 0;
    _size = osc_buf_size;
    return true;
}

auto COscilloscope::getDecimation() -> uint32_t {
    return m_dec_factor;
}

auto COscilloscope::getOSCRate() -> uint32_t{
    return m_dec_factor;
}

auto COscilloscope::clearBuffer() -> bool{
    return true;
}

auto COscilloscope::wait() -> bool{
    return true;
}

auto COscilloscope::clearInterrupt() -> bool{
    return true;
}

auto isMaster() -> BoardMode{
    return BoardMode::UNKNOWN;
}

auto COscilloscope::stop() -> void{}

auto COscilloscope::printReg() -> void{}

auto COscilloscope::set8BitMode(bool) -> void{}
