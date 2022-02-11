#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "Oscilloscope.h"
#include <stdio.h>
#include <string.h>
#include <poll.h>

#define UNUSED(x) [&x]{}()

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

void setRegister(volatile OscilloscopeMapT * baseOsc_addr,volatile uint32_t *reg, int32_t value){
    UNUSED(baseOsc_addr);
//    fprintf(stderr,"\tSet register 0x%X <- 0x%X\n",(uint32_t)reg-(uint32_t)baseOsc_addr,value);
    *reg = value;
}


}

COscilloscope::Ptr COscilloscope::Create(const UioT &_uio, bool _channel1Enable, bool _channel2Enable,uint32_t _dec_factor,bool _isMaster)
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

   
    return std::make_shared<COscilloscope>(_channel1Enable,_channel2Enable, fd, regset, _uio.mapList[0].size, buffer, _uio.mapList[1].size, _uio.mapList[1].addr,_dec_factor,_isMaster);
}

COscilloscope::COscilloscope(bool _channel1Enable, bool _channel2Enable, int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t _dec_factor,bool _isMaster) :
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
    m_dec_factor(_dec_factor),
    m_filterBypass(true),
    m_isMaster(_isMaster)
{
    m_calib_offset_ch1 = 0;
    m_calib_gain_ch1 = 0x8000;
    m_calib_offset_ch2 = 0;
    m_calib_gain_ch2 = 0x8000;
    setFilterCalibrationCh1(0,0,0xFFFFFF,0);
    setFilterCalibrationCh2(0,0,0xFFFFFF,0);
    uintptr_t oscMap = reinterpret_cast<uintptr_t>(m_Regset);
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

void COscilloscope::setReg(volatile OscilloscopeMapT *_OscMap){
        // Buffer
        setRegister(_OscMap,&(_OscMap->dma_buf_size),osc_buf_size);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr1_ch1),m_BufferPhysAddr);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr2_ch1),m_BufferPhysAddr + osc_buf_size);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr1_ch2),m_BufferPhysAddr + osc_buf_size * 2);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr2_ch2),m_BufferPhysAddr + osc_buf_size * 3);
        
    
        setRegister(_OscMap,&(_OscMap->filt_bypass), m_filterBypass ? UINT32_C(0x00000001) : UINT32_C(0x00000000));
        //_OscMap->filt_bypass = UINT32_C(0x00000001);
        
        // Event
        setRegister(_OscMap,&(_OscMap->event_sel),osc0_event_id);
        //_OscMap->event_sel =  osc0_event_id ;

        // Trigger mask
        setRegister(_OscMap,&(_OscMap->trig_mask),m_isMaster ? UINT32_C(0x00000004) : UINT32_C(0x00000020));
        //setRegister(_OscMap,&(_OscMap->trig_mask), UINT32_C(0x00000004));

        // Trigger low level
        setRegister(_OscMap,&(_OscMap->trig_low_level),-4);
        //_OscMap->trig_low_level = -4;

        // Trigger high level
        setRegister(_OscMap,&(_OscMap->trig_high_level),4);
        //_OscMap->trig_high_level = 4;

        // Trigger edge
        setRegister(_OscMap,&(_OscMap->trig_edge),UINT32_C(0x00000000));
        //_OscMap->trig_edge = UINT32_C(0x00000000);

        // Trigger pre samples
        setRegister(_OscMap,&(_OscMap->trig_pre_samp),m_isMaster ? osc_buf_pre_samp : 0);

        //_OscMap->trig_pre_samp = osc_buf_pre_samp;

        // Trigger post samples
        setRegister(_OscMap,&(_OscMap->trig_post_samp),osc_buf_post_samp);            
        // _OscMap->trig_post_samp = osc_buf_post_samp;

        // Decimate factor
        setRegister(_OscMap,&(_OscMap->dec_factor),m_dec_factor);   
        //_OscMap->dec_factor = m_dec_factor; 

        setRegister(_OscMap,&(_OscMap->calib_offset_ch1),m_calib_offset_ch1);

        setRegister(_OscMap,&(_OscMap->calib_gain_ch1),m_calib_gain_ch1);

        setRegister(_OscMap,&(_OscMap->calib_offset_ch2),m_calib_offset_ch2);

        setRegister(_OscMap,&(_OscMap->calib_gain_ch2),m_calib_gain_ch2);

#ifndef Z20_250_12
        setRegister(_OscMap,&(_OscMap->filt_coeff_aa_ch1),m_AA_ch1);

        setRegister(_OscMap,&(_OscMap->filt_coeff_bb_ch1),m_BB_ch1);

        setRegister(_OscMap,&(_OscMap->filt_coeff_kk_ch1),m_KK_ch1);

        setRegister(_OscMap,&(_OscMap->filt_coeff_pp_ch1),m_PP_ch1);

        setRegister(_OscMap,&(_OscMap->filt_coeff_aa_ch2),m_AA_ch2);

        setRegister(_OscMap,&(_OscMap->filt_coeff_bb_ch2),m_BB_ch2);

        setRegister(_OscMap,&(_OscMap->filt_coeff_kk_ch2),m_KK_ch2);

        setRegister(_OscMap,&(_OscMap->filt_coeff_pp_ch2),m_PP_ch2);
#endif
}

void COscilloscope::setFilterCalibrationCh1(int32_t _aa,int32_t _bb, int32_t _kk, int32_t _pp){
    m_AA_ch1 = _aa;
    m_BB_ch1 = _bb;
    m_KK_ch1 = _kk;
    m_PP_ch1 = _pp;
}

void COscilloscope::setFilterCalibrationCh2(int32_t _aa,int32_t _bb, int32_t _kk, int32_t _pp){
    m_AA_ch2 = _aa;
    m_BB_ch2 = _bb;
    m_KK_ch2 = _kk;
    m_PP_ch2 = _pp;
}

void COscilloscope::setFilterBypass(bool _state){
    m_filterBypass = _state;
}

void COscilloscope::prepare()
{
    stop();

    if (m_OscMap != nullptr){
        setReg(m_OscMap);
    }else{
        std::cerr << "Error: COscilloscope::prepare()  can't init first channel" << std::endl;
        exit(-1);
    }
    setRegister(m_OscMap,&(m_OscMap->dma_ctrl) , UINT32_C(0x0000021E));
    setRegister(m_OscMap,&(m_OscMap->event_sts),UINT32_C(0x00000002));

    if (m_isMaster){
        setRegister(m_OscMap,&(m_OscMap->dma_ctrl) ,UINT32_C(0x00000001));
    }
}

void COscilloscope::setCalibration(int32_t ch1_offset,float ch1_gain, int32_t ch2_offset, float ch2_gain){
    if (ch1_gain >= 2) ch1_gain = 1.999999;
    if (ch1_gain < 0)  ch1_gain = 0;
    if (ch2_gain >= 2) ch2_gain = 1.999999;
    if (ch2_gain < 0)  ch2_gain = 0;

    m_calib_offset_ch1 =  ch1_offset * -4;
    m_calib_offset_ch2 =  ch2_offset * -4;
    m_calib_gain_ch1 = ch1_gain * 32768; 
    m_calib_gain_ch2 = ch2_gain * 32768; 
}

bool COscilloscope::next(uint8_t *&_buffer1,uint8_t *&_buffer2, size_t &_size,uint32_t &_overFlow)
{
    // Interrupt ACQ
    _buffer1 = m_Channel1 ? ( m_OscBuffer1 + osc_buf_size * m_OscBufferNumber) : nullptr;
    _buffer2 = m_Channel2 ? ( m_OscBuffer2 + osc_buf_size * m_OscBufferNumber) : nullptr;
    // This fix for FPGA
    _overFlow = (m_OscBufferNumber == 1 ? m_OscMap->lost_samples_buf1 : m_OscMap->lost_samples_buf2);
    if (m_Channel1 || m_Channel2){
        _size = osc_buf_size;
    }else {
        _size = 0;
    }
    return true;    
}



bool COscilloscope::clearBuffer(){
    uint32_t clearFlag = (m_OscBufferNumber == 0 ? 0x00000004 : 0x00000008); // reset buffer
    setRegister(m_OscMap,&(m_OscMap->dma_ctrl), clearFlag );
    if (m_Channel1 || m_Channel2){
        m_OscBufferNumber = (m_OscBufferNumber == 0) ? 1 : 0;
    }
    return true;
}

bool COscilloscope::wait(){
    int32_t cnt = 1;
    constexpr size_t cnt_size = sizeof(cnt);
    ssize_t bytes = write(m_Fd, &cnt, cnt_size); // Unmmask interrupt
    if (bytes == cnt_size) {
        // wait for the interrupt
//        printf("Wait Itr\n");
        struct pollfd pfd = {.fd = m_Fd, .events = POLLIN};
        int timeout_ms = 1000;
        int rv = poll(&pfd, 1, timeout_ms);
        // clear the interrupt
        if (rv >= 1) {
               uint32_t info;
               read(m_Fd, &info, sizeof(info));
//               printf("Itr\n");
        } else if (rv == 0) {
               return false;
        } else {
               perror("UIO::wait()");
        }
        clearInterrupt();
        return true;
    }
    return false;
}

bool COscilloscope::clearInterrupt(){
    const std::lock_guard<std::mutex> lock(m_waitLock);
    // fprintf(stderr,"clearInterrupt()\n");
    setRegister(m_OscMap,&(m_OscMap->dma_ctrl), 0x00000002 );
    return true;
}

void COscilloscope::stop()
{
    const std::lock_guard<std::mutex> lock(m_waitLock);
    if (m_OscMap != nullptr){
        setRegister(m_OscMap,&(m_OscMap->event_sts),UINT32_C(0x00000004));
    }else {
        std::cerr << "Error: COscilloscope::stop()" << std::endl;
        exit(-1);
    }
}

auto COscilloscope::printReg() -> void{
    #ifdef Z20_250_12
        fprintf(stderr,"not implemented\n");
    #else
        fprintf(stderr,"printReg\n");
        fprintf(stderr,"0x00 event_sts = 0x%X\n", m_OscMap->event_sts);
        fprintf(stderr,"0x04 event_sel = 0x%X\n", m_OscMap->event_sel);
        fprintf(stderr,"0x08 trig_mask = 0x%X\n", m_OscMap->trig_mask);
        fprintf(stderr,"0x10 trig_pre_samp = 0x%X\n", m_OscMap->trig_pre_samp);
        fprintf(stderr,"0x14 trig_post_samp = 0x%X\n", m_OscMap->trig_post_samp);
        fprintf(stderr,"0x18 trig_pre_cnt = 0x%X\n", m_OscMap->trig_pre_cnt);
        fprintf(stderr,"0x1C trig_post_cnt = 0x%X\n", m_OscMap->trig_post_cnt);
        fprintf(stderr,"0x20 trig_low_level = 0x%X\n", m_OscMap->trig_low_level);
        fprintf(stderr,"0x24 trig_high_level = 0x%X\n", m_OscMap->trig_high_level);
        fprintf(stderr,"0x28 trig_edge = 0x%X\n", m_OscMap->trig_edge);
        
        fprintf(stderr,"0x30 dec_factor = 0x%X\n", m_OscMap->dec_factor);
        fprintf(stderr,"0x34 dec_rshift = 0x%X\n", m_OscMap->dec_rshift);
        fprintf(stderr,"0x38 avg_en_addr = 0x%X\n", m_OscMap->avg_en_addr);
        fprintf(stderr,"0x3C filt_bypass = 0x%X\n", m_OscMap->filt_bypass);

        fprintf(stderr,"0x50 dma_ctrl = 0x%X\n", m_OscMap->dma_ctrl);
        fprintf(stderr,"0x54 dma_sts_addr = 0x%X\n", m_OscMap->dma_sts_addr);
        fprintf(stderr,"0x58 dma_buf_size = 0x%X\n", m_OscMap->dma_buf_size);
        fprintf(stderr,"0x5C lost_samples_buf1 = 0x%X\n", m_OscMap->lost_samples_buf1);
        fprintf(stderr,"0x60 lost_samples_buf2 = 0x%X\n", m_OscMap->lost_samples_buf2);
        fprintf(stderr,"0x64 dma_dst_addr1_ch1 = 0x%X\n", m_OscMap->dma_dst_addr1_ch1);
        fprintf(stderr,"0x68 dma_dst_addr2_ch1 = 0x%X\n", m_OscMap->dma_dst_addr2_ch1);
        fprintf(stderr,"0x6C dma_dst_addr1_ch2 = 0x%X\n", m_OscMap->dma_dst_addr1_ch2);
        fprintf(stderr,"0x70 dma_dst_addr2_ch2 = 0x%X\n", m_OscMap->dma_dst_addr2_ch2);

        fprintf(stderr,"0x74 calib_offset_ch1 = 0x%X\n", m_OscMap->calib_offset_ch1);
        fprintf(stderr,"0x78 calib_gain_ch1 = 0x%X\n", m_OscMap->calib_gain_ch1);
        fprintf(stderr,"0x7C calib_offset_ch2 = 0x%X\n", m_OscMap->calib_offset_ch2);
        fprintf(stderr,"0x80 calib_gain_ch2 = 0x%X\n", m_OscMap->calib_gain_ch2);

        fprintf(stderr,"0x9C lost_samples_buf1_ch2 = 0x%X\n", m_OscMap->lost_samples_buf1_ch2);
        fprintf(stderr,"0xA0 lost_samples_buf2_ch2 = 0x%X\n", m_OscMap->lost_samples_buf2_ch2);
        fprintf(stderr,"0xA4 write_pointer_ch1 = 0x%X\n", m_OscMap->write_pointer_ch1);
        fprintf(stderr,"0xA8 write_pointer_ch2 = 0x%X\n", m_OscMap->write_pointer_ch2);

        fprintf(stderr,"0xC0 filt_coeff_aa_ch1 = 0x%X\n", m_OscMap->filt_coeff_aa_ch1);
        fprintf(stderr,"0xC4 filt_coeff_bb_ch1 = 0x%X\n", m_OscMap->filt_coeff_bb_ch1);
        fprintf(stderr,"0xC8 filt_coeff_kk_ch1 = 0x%X\n", m_OscMap->filt_coeff_kk_ch1);
        fprintf(stderr,"0xCC filt_coeff_pp_ch1 = 0x%X\n", m_OscMap->filt_coeff_pp_ch1);
        fprintf(stderr,"0xD0 filt_coeff_aa_ch2 = 0x%X\n", m_OscMap->filt_coeff_aa_ch2);
        fprintf(stderr,"0xD4 filt_coeff_bb_ch2 = 0x%X\n", m_OscMap->filt_coeff_bb_ch2);
        fprintf(stderr,"0xD8 filt_coeff_kk_ch2 = 0x%X\n", m_OscMap->filt_coeff_kk_ch2);
        fprintf(stderr,"0xDC filt_coeff_pp_ch2 = 0x%X\n", m_OscMap->filt_coeff_pp_ch2);

    #endif
}
