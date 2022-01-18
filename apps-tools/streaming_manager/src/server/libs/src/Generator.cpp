#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "Generator.h"
#include <stdio.h>
#include <string.h>

#define UNUSED(x) [&x]{}()

void * MmapNumber(int _fd, size_t _size, size_t _number) {
    const size_t offset = _number * getpagesize();
    return mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, offset);
}

inline void setRegister(volatile GeneratorMapT * baseOsc_addr,volatile uint32_t *reg, int32_t value){
    UNUSED(baseOsc_addr);
//    fprintf(stderr,"\tSet register 0x%X <- 0x%X\n",(uint32_t)reg-(uint32_t)baseOsc_addr,value);
    *reg = value;
}

CGenerator::Ptr CGenerator::Create(const UioT &_uio, bool _channel1Enable, bool _channel2Enable,uint32_t dacHz,uint32_t maxDacHz)
{
    // Validation
    if (_uio.mapList.size() < 2)
    {
        // Error: validation.
        std::cerr << "Error: UIO validation." << std::endl;
        return CGenerator::Ptr();
    }

    if (_uio.mapList[1].size < (dac_buf_size * 4))
    {
        // Error: buffer size.
        std::cerr << "Error: buffer size." << std::endl;
        return CGenerator::Ptr();
    }

    // Open file
    std::string path("/dev/" + _uio.name);
    int fd = open(path.c_str(), O_RDWR);

    if (fd == -1)
    {
        // Error: open file.
        std::cerr << "Error: open file." << std::endl;
        return CGenerator::Ptr();
    }

    // Map
    void *regset = MmapNumber(fd, _uio.mapList[0].size, 0);

    if (regset == MAP_FAILED)
    {
        // Error: mmap
        std::cerr << "Error: mmap regset." << std::endl;
        close(fd);
        return CGenerator::Ptr();
    }

    void *buffer = MmapNumber(fd, _uio.mapList[1].size, 1);

    if (buffer == MAP_FAILED)
    {
        // Error: mmap
        std::cerr << "Error: mmap buffer." << std::endl;
        munmap(regset, _uio.mapList[0].size);
        close(fd);
        return CGenerator::Ptr();
    }

   
    return std::make_shared<CGenerator>(_channel1Enable,_channel2Enable, fd, regset, _uio.mapList[0].size, buffer, _uio.mapList[1].size, _uio.mapList[1].addr,dacHz,maxDacHz);
}

CGenerator::CGenerator(bool _channel1Enable, bool _channel2Enable, int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t dacHz,uint32_t maxDacHz) :
    m_Channel1(_channel1Enable),
    m_Channel2(_channel2Enable),
    m_Fd(_fd),
    m_Regset(_regset),
    m_RegsetSize(_regsetSize),
    m_Buffer(_buffer),
    m_BufferSize(_bufferSize),
    m_BufferPhysAddr(_bufferPhysAddr),
    m_Map(nullptr),
    m_Buffer1(nullptr),
    m_Buffer2(nullptr),
    m_waitLock(),
    m_maxDacSpeedHz(maxDacHz),
    m_dacSpeedHz(dacHz)
{
    m_BufferNumber[0] = m_BufferNumber[1] = 0;
    m_calib_offset_ch1 = 0;
    m_calib_gain_ch1 = 0x2000;
    m_calib_offset_ch2 = 0;
    m_calib_gain_ch2 = 0x2000;
    uintptr_t Map = reinterpret_cast<uintptr_t>(m_Regset);
    m_Map = reinterpret_cast<GeneratorMapT *>(Map);
    m_Buffer1 = static_cast<uint8_t *>(m_Buffer);
    m_Buffer2 = static_cast<uint8_t *>(m_Buffer) + dac_buf_size * 2;
    memset(m_Buffer,0,dac_buf_size * 4);
}

CGenerator::~CGenerator()
{
    stop();
    munmap(m_Regset, m_RegsetSize);
    munmap(m_Buffer, m_BufferSize);
    close(m_Fd);
}

auto CGenerator::getDacHz() -> uint32_t{
    return m_dacSpeedHz;
}

auto CGenerator::setDacHz(uint32_t hz) -> bool{
    if (((double)hz / (double)m_maxDacSpeedHz) * (1<<16) < 1) return false;
    m_dacSpeedHz = hz;
    return true;
}

void CGenerator::setReg(volatile GeneratorMapT *_Map){

        // Reset EVENT        
        setRegister(_Map,&(_Map->event_status),0);
        // Buffer
        setRegister(_Map,&(_Map->dma_size),dac_buf_size);
        setRegister(_Map,&(_Map->chA_dma_addr1),m_BufferPhysAddr);
        setRegister(_Map,&(_Map->chA_dma_addr2),m_BufferPhysAddr + dac_buf_size);
        setRegister(_Map,&(_Map->chB_dma_addr1),m_BufferPhysAddr + dac_buf_size * 2);
        setRegister(_Map,&(_Map->chB_dma_addr2),m_BufferPhysAddr + dac_buf_size * 3);

        // Select event
        setRegister(_Map,&(_Map->event_select),gen0_event_id);

        // Event trig
        setRegister(_Map,&(_Map->trig_mask),1);

        // Set trigger immediatly
        setRegister(_Map,&(_Map->config),0x10001);

        // Set calib for chA
        setRegister(_Map,&(_Map->chA_calib),m_calib_offset_ch1 << 16 | m_calib_gain_ch1);

        // Set calib for chB
        setRegister(_Map,&(_Map->chB_calib),m_calib_offset_ch2 << 16 | m_calib_gain_ch2);

        // Set step for pointer
        double coff = (double)m_dacSpeedHz / (double)m_maxDacSpeedHz;
        uint32_t step = (1 << 16) * coff; 
        setRegister(_Map,&(_Map->chA_counter_step),step);
        setRegister(_Map,&(_Map->chB_counter_step),step);

        // Set streaming DMA, reset Buffers and flags
        setRegister(_Map,&(_Map->dma_control),0x2222);
}

auto CGenerator::prepare() -> void
{
    stop();

    if (m_Map != nullptr){
        setReg(m_Map);
    }else{
        std::cerr << "Error: CGenerator::prepare()  can't init first channel" << std::endl;
        exit(-1);
    }
    m_BufferNumber[0] = m_BufferNumber[1] = 0;

}

auto CGenerator::setCalibration(int32_t ch1_offset,float ch1_gain, int32_t ch2_offset, float ch2_gain) -> void{
    if (ch1_gain >= 2) ch1_gain = 1.999999;
    if (ch1_gain < 0)  ch1_gain = 0;
    if (ch2_gain >= 2) ch2_gain = 1.999999;
    if (ch2_gain < 0)  ch2_gain = 0;

    m_calib_offset_ch1 =  ch1_offset;
    m_calib_offset_ch2 =  ch2_offset;

    m_calib_gain_ch1 = ch1_gain * 0x2000;
    m_calib_gain_ch2 = ch2_gain * 0x2000;
}

auto CGenerator::initFirst(uint8_t *_buffer1,uint8_t *_buffer2, size_t _size_ch1, size_t _size_ch2) -> bool{
    const std::lock_guard<std::mutex> lock(m_waitLock);
    bool ret = false;
    if (_buffer1){
        memcpy_neon(m_Buffer1,_buffer1,_size_ch1);
        setRegister(m_Map,&(m_Map->dma_control),1 << 6);
        ret = true;
    }

    if (_buffer2){
        memcpy_neon(m_Buffer2,_buffer2,_size_ch2);
        setRegister(m_Map,&(m_Map->dma_control),1 << 14);
        ret = true;
    }
    return ret;
}

auto CGenerator::initSecond(uint8_t *_buffer1,uint8_t *_buffer2, size_t _size_ch1, size_t _size_ch2) -> bool{
    const std::lock_guard<std::mutex> lock(m_waitLock);
    bool ret = false;
    if (_buffer1){
        memcpy_neon((&(*m_Buffer1)+dac_buf_size),_buffer1,_size_ch1);
        setRegister(m_Map,&(m_Map->dma_control),1 << 7);
        ret = true;
    }

    if (_buffer2){
        memcpy_neon((&(*m_Buffer2)+dac_buf_size),_buffer2,_size_ch2);
        setRegister(m_Map,&(m_Map->dma_control),1 << 15);
        ret = true;
    }
    return ret;
}

auto CGenerator::write(uint8_t *_buffer1,uint8_t *_buffer2, size_t _size_ch1, size_t _size_ch2) -> bool
{
    bool ret = false;
    const std::lock_guard<std::mutex> lock(m_waitLock);
    auto status = m_Map->ch_dma_status;
    if (m_BufferNumber[0] == 0){
        if (status & 0x00030000 && status & 0x00000003){
            // printReg();
	        if (_buffer1 && _size_ch1) memcpy_neon((&(*m_Buffer1)+dac_buf_size),_buffer1,_size_ch1);
            if (_buffer2 && _size_ch2) memcpy_neon((&(*m_Buffer2)+dac_buf_size),_buffer2,_size_ch2);
            m_BufferNumber[0] = 1;
            int command =  1 << 7  |  1 << 15;
            setRegister(m_Map,&(m_Map->dma_control),command);
            ret = true;
        }
    }else{
        if (status & 0x000C0000 && status & 0x0000000C){
            // printReg();
            if (_buffer1 && _size_ch1) memcpy_neon(m_Buffer1,_buffer1,_size_ch1);
            if (_buffer2 && _size_ch2) memcpy_neon(m_Buffer2,_buffer2,_size_ch2);
            m_BufferNumber[0] = 0;
            int command =  1 << 6  |  1 << 14;
            setRegister(m_Map,&(m_Map->dma_control),command);
            ret = true;
        }
    }
    return ret;
}


auto CGenerator::start() -> void{
    const std::lock_guard<std::mutex> lock(m_waitLock);
    if (m_Map != nullptr){
        // Start event
        setRegister(m_Map,&(m_Map->event_status),0x2);

        // Start DMA
       setRegister(m_Map,&(m_Map->dma_control),0x101);
    }else {
        std::cerr << "Error: CGenerator::stop()" << std::endl;
        exit(-1);
    }
}

auto CGenerator::stop() -> void
{
    const std::lock_guard<std::mutex> lock(m_waitLock);
    if (m_Map != nullptr){
         // Stop EVENT        
        setRegister(m_Map,&(m_Map->event_status),0x4);
        // Stop DMA        
        setRegister(m_Map,&(m_Map->dma_control),0);
    }else {
        std::cerr << "Error: CGenerator::stop()" << std::endl;
        exit(-1);
    }
}

auto CGenerator::printReg() -> void{
    fprintf(stderr,"printReg\n");
    fprintf(stderr,"0x00 Config = 0x%X\n", m_Map->config);
    fprintf(stderr,"0x04 chA_calib = 0x%X\n", m_Map->chA_calib);
    fprintf(stderr,"0x08 chA_counter_step = 0x%X\n", m_Map->chA_counter_step);
    fprintf(stderr,"0x0C chA_read_pointer = 0x%X\n", m_Map->chA_read_pointer);
    fprintf(stderr,"0x10 chB_calib = 0x%X\n", m_Map->chB_calib);
    fprintf(stderr,"0x14 chB_counter_step = 0x%X\n", m_Map->chB_counter_step);
    fprintf(stderr,"0x18 chB_read_pointer = 0x%X\n", m_Map->chB_read_pointer);
    fprintf(stderr,"0x1C event_status = 0x%X\n", m_Map->event_status);
    fprintf(stderr,"0x20 event_select = 0x%X\n", m_Map->event_select);
    fprintf(stderr,"0x24 trig_mask = 0x%X\n", m_Map->trig_mask);
    fprintf(stderr,"0x28 dma_control = 0x%X\n", m_Map->dma_control);
    fprintf(stderr,"0x2C ch_dma_status = 0x%X\n", m_Map->ch_dma_status);
    fprintf(stderr,"0x34 dma_size = 0x%X\n", m_Map->dma_size);
    fprintf(stderr,"0x38 chA_dma_addr1 = 0x%X\n", m_Map->chA_dma_addr1);
    fprintf(stderr,"0x3C chA_dma_addr2 = 0x%X\n", m_Map->chA_dma_addr2);
    fprintf(stderr,"0x40 chB_dma_addr1 = 0x%X\n", m_Map->chB_dma_addr1);
    fprintf(stderr,"0x44 chB_dma_addr2 = 0x%X\n", m_Map->chB_dma_addr2);

}
