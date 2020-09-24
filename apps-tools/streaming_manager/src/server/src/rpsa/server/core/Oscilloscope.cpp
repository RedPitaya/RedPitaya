#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "rpsa/server/core/Oscilloscope.h"
#include <stdio.h>
#include <string.h>

namespace
{
//!
//!@brief Map register using UIO.
//!
//!@param _fd File descriptor.
//!@param _size The register map size.
//!@param _number The register map number.
//!@return The memory map pointer.
//!
void * MmapNumber(int _fd, size_t _size, size_t _number) {
    const size_t offset = _number * getpagesize();
    return mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, offset);
}

void setRegister(uint32_t baseOsc_addr,volatile uint32_t *reg, int32_t value){
    //printf("\tSet register 0x%X <- 0x%X\n",(uint32_t)reg-baseOsc_addr,value);
    *reg = value;
}
}

COscilloscope::Ptr COscilloscope::Create(const UioT &_uio, bool _channel1Enable, bool _channel2Enable,uint32_t _dec_factor)
{
    // Validation
    if (_uio.mapList.size() < 2)
    {
        // Error: validation.
        std::cerr << "Error: UIO validation." << std::endl;
        return COscilloscope::Ptr();
    }

    if (_uio.mapList[1].size < (osc_buf_size * 4))
    {
        // Error: buffer size.
        std::cerr << "Error: buffer size." << std::endl;
        return COscilloscope::Ptr();
    }

    // Open file
    std::string path("/dev/" + _uio.name);
    int fd = open(path.c_str(), O_RDWR);

    if (fd == -1)
    {
        // Error: open file.
        std::cerr << "Error: open file." << std::endl;
        return COscilloscope::Ptr();
    }

    // Map
    void *regset = MmapNumber(fd, _uio.mapList[0].size, 0);

    if (regset == MAP_FAILED)
    {
        // Error: mmap
        std::cerr << "Error: mmap regset." << std::endl;
        close(fd);
        return COscilloscope::Ptr();
    }

    void *buffer = MmapNumber(fd, _uio.mapList[1].size, 1);

    if (buffer == MAP_FAILED)
    {
        // Error: mmap
        std::cerr << "Error: mmap buffer." << std::endl;
        munmap(regset, _uio.mapList[0].size);
        close(fd);
        return COscilloscope::Ptr();
    }

   
    return std::make_shared<COscilloscope>(_channel1Enable,_channel2Enable, fd, regset, _uio.mapList[0].size, buffer, _uio.mapList[1].size, _uio.mapList[1].addr,_dec_factor);
}

COscilloscope::COscilloscope(bool _channel1Enable, bool _channel2Enable, int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t _dec_factor) :
    m_Channel1(_channel1Enable),
    m_Channel2(_channel2Enable),
    m_Fd(_fd),
    m_Regset(_regset),
    m_RegsetSize(_regsetSize),
    m_Buffer(_buffer),
    m_BufferSize(_bufferSize),
    m_BufferPhysAddr(_bufferPhysAddr),
    m_OscMap(nullptr),
    m_OscBuffer1(nullptr),
    m_OscBuffer2(nullptr),
    m_OscBufferNumber(0),
    m_dec_factor(_dec_factor)
{
    uintptr_t oscMap = reinterpret_cast<uintptr_t>(m_Regset) +  osc0_baseaddr ;
    m_OscMap = reinterpret_cast<OscilloscopeMapT *>(oscMap);
    m_OscBuffer1 = static_cast<uint8_t *>(m_Buffer);  
    m_OscBuffer2 = static_cast<uint8_t *>(m_Buffer) + osc_buf_size * 2;
    
}

COscilloscope::~COscilloscope()
{
    munmap(m_Regset, m_RegsetSize);
    munmap(m_Buffer, m_BufferSize);
    close(m_Fd);
}

void COscilloscope::setReg(volatile OscilloscopeMapT *_OscMap,unsigned int _Channel){
        // Buffer
        setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_buf_size),osc_buf_size);
        setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_dst_addr1_ch1),m_BufferPhysAddr);
        setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_dst_addr2_ch1),m_BufferPhysAddr + osc_buf_size);
        setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_dst_addr1_ch2),m_BufferPhysAddr + osc_buf_size * 2);
        setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_dst_addr2_ch2),m_BufferPhysAddr + osc_buf_size * 3);
        
    
        setRegister((uint32_t)m_OscMap,&(m_OscMap->filt_bypass),UINT32_C(0x00000001));
        //_OscMap->filt_bypass = UINT32_C(0x00000001);

        
        // Event
        setRegister((uint32_t)m_OscMap,&(m_OscMap->event_sel),osc0_event_id);
        //_OscMap->event_sel =  osc0_event_id ;

        // Trigger mask
        setRegister((uint32_t)m_OscMap,&(m_OscMap->trig_mask),UINT32_C(0x00000004));
        //_OscMap->trig_mask = UINT32_C(0x00000004);

        // Trigger low level
        setRegister((uint32_t)m_OscMap,&(m_OscMap->trig_low_level),-4);
        //_OscMap->trig_low_level = -4;

        // Trigger high level
        setRegister((uint32_t)m_OscMap,&(m_OscMap->trig_high_level),4);
        //_OscMap->trig_high_level = 4;

        // Trigger edge
        setRegister((uint32_t)m_OscMap,&(m_OscMap->trig_edge),UINT32_C(0x00000000));
        //_OscMap->trig_edge = UINT32_C(0x00000000);

        // Trigger pre samples
        setRegister((uint32_t)m_OscMap,&(m_OscMap->trig_pre_samp),osc_buf_pre_samp);
        //_OscMap->trig_pre_samp = osc_buf_pre_samp;

        // Trigger post samples
        setRegister((uint32_t)m_OscMap,&(m_OscMap->trig_post_samp),osc_buf_post_samp);            
        // _OscMap->trig_post_samp = osc_buf_post_samp;

        // Decimate factor
        setRegister((uint32_t)m_OscMap,&(m_OscMap->dec_factor),m_dec_factor);   
        //_OscMap->dec_factor = m_dec_factor; 
        
}

void COscilloscope::prepare()
{
    stop();

    if (m_OscMap != nullptr){
        setReg(m_OscMap,0);
    }else{
        std::cerr << "Error: COscilloscope::prepare()  can't init first channel" << std::endl;
        exit(-1);
    }
    //printf("prepare()\n");
    setRegister((uint32_t)m_OscMap,&(m_OscMap->event_sts),UINT32_C(0x00000004));
    setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_ctrl) ,UINT32_C(0x00000212));
    setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_ctrl) ,UINT32_C(0x00000001));
    setRegister((uint32_t)m_OscMap,&(m_OscMap->event_sts),UINT32_C(0x00000002));
    
}

bool COscilloscope::next(uint8_t *&_buffer1,uint8_t *&_buffer2, size_t &_size,uint32_t &_overFlow)
{
    //printf("getBuffer(%d);\n",m_OscBufferNumber);
    // Interrupt ACQ
    //printf("\tDMA status  0x%X\n",m_OscMap->dma_sts_addr);
    //printf("\tDMA lost1:  0x%X\tlost2:  0x%X\n",m_OscMap->lost_samples_buf1,m_OscMap->lost_samples_buf2);
    //printf("\tCurrent buffer %d\n",m_OscBufferNumber);
    _buffer1 = m_Channel1 ? ( m_OscBuffer1 + osc_buf_size * m_OscBufferNumber) : nullptr;
    _buffer2 = m_Channel2 ? ( m_OscBuffer2 + osc_buf_size * m_OscBufferNumber) : nullptr;
    _overFlow = (m_OscBufferNumber == 0 ? m_OscMap->lost_samples_buf1 : m_OscMap->lost_samples_buf2);

    if (m_Channel1 || m_Channel2){
        _size = osc_buf_size;
    }else {
        _size = 0;
    }
    return true;    
}



bool COscilloscope::clearBuffer(){
//    printf("clearBuffer(%d)\n",m_OscBufferNumber);
    uint32_t clearFlag = (m_OscBufferNumber == 0 ? 0x00000004 : 0x00000008); // reset buffer
    setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_ctrl), clearFlag );
 
    if (m_Channel1 || m_Channel2){
        m_OscBufferNumber = (m_OscBufferNumber == 0) ? 1 : 0;
    }
    return true;
}

bool COscilloscope::wait(){
    int32_t cnt = 1;
    constexpr size_t cnt_size = sizeof(cnt);
    ssize_t bytes = write(m_Fd, &cnt, cnt_size);
//    printf("wait()\n");
    if (bytes == cnt_size) {

//        printf("\tWAIT INTERRUPT\n");
        bytes = read(m_Fd, &cnt, cnt_size);
//        printf("\tINTERRUPT GET\n");
        clearInterrupt();
        if (bytes == cnt_size) {
            return true;
        }
    }
//    printf("Exit by false\n");
    return false;
}

bool COscilloscope::clearInterrupt(){
//    printf("clearInterrupt()\n");
    setRegister((uint32_t)m_OscMap,&(m_OscMap->dma_ctrl), 0x00000002 );
    return true;
}

void COscilloscope::stop()
{
    // Control stop
//    printf("stop()\n");
    if (m_OscMap != nullptr){
        setRegister((uint32_t)m_OscMap,&(m_OscMap->event_sts),UINT32_C(0x00000004));
    }else {
        std::cerr << "Error: COscilloscope::stop()" << std::endl;
        exit(-1);
    }
}
