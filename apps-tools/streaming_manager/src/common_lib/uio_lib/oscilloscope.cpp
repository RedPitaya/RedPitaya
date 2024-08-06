#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "oscilloscope.h"
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <math.h>
#include "logger_lib/file_logger.h"

using namespace uio_lib;

void * MmapNumber(int _fd, size_t _size, size_t _number) {
    const size_t offset = _number * getpagesize();
    return mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, offset);
}

void setRegister(volatile OscilloscopeMapT * baseOsc_addr,volatile uint32_t *reg, int32_t value){
    (void)(baseOsc_addr);
    //fprintf(stderr,"\tSet register 0x%X <- 0x%X\n",(uint32_t)reg-(uint32_t)baseOsc_addr,value);
    *reg = value;
}

auto COscilloscope::create(const UioT &_uio,uint32_t _dec_factor,bool _isMaster,uint32_t _adcMaxSpeed,bool _isADCFilterPresent,uint8_t _fpgaBits,uint8_t _maxChannels) -> COscilloscope::Ptr{
    // Validation
    if (_uio.mapList.size() < 2)
    {
        printf("map list %d\n",_uio.mapList.size());
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


    return std::make_shared<COscilloscope>(fd, regset, _uio.mapList[0].size, buffer, _uio.mapList[1].size, _uio.mapList[1].addr,_dec_factor,_isMaster,_adcMaxSpeed,_isADCFilterPresent,_fpgaBits,_maxChannels);
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
    m_waitLock(),
    m_filterBypass(true),
    m_isMaster(_isMaster),
    m_adcMaxSpeed(_adcMaxSpeed),
    m_isADCFilterPresent(_isADCFilterPresent),
    m_fpgaBits(_fpgaBits),
    m_maxChannels(_maxChannels)
{
    for(int i = 0; i < 4; i++){
        m_OscBuffer[i] = nullptr;
        setCalibration(i,0,1.0);
        setFilterCalibration(i,0,0,0xFFFFFF,0);
    }

    uintptr_t oscMap = reinterpret_cast<uintptr_t>(m_Regset);
    m_OscMap = reinterpret_cast<OscilloscopeMapT *>(oscMap);
    for(auto j = 0u; j < m_maxChannels; j++){
        m_OscBuffer[j] = (m_maxChannels > j ? static_cast<uint8_t *>(m_Buffer) + (osc_buf_size * 2 * j )  : nullptr);
    }
}

COscilloscope::~COscilloscope()
{
    munmap(m_Regset, m_RegsetSize);
    munmap(m_Buffer, m_BufferSize);
    close(m_Fd);
    TRACE("Exit")
}

auto COscilloscope::setReg(volatile OscilloscopeMapT *_OscMap) -> void {
        // Buffer
        setRegister(_OscMap,&(_OscMap->dma_buf_size),osc_buf_size);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr1_ch1),m_BufferPhysAddr);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr2_ch1),m_BufferPhysAddr + osc_buf_size);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr1_ch2),m_BufferPhysAddr + osc_buf_size * 2);
        setRegister(_OscMap,&(_OscMap->dma_dst_addr2_ch2),m_BufferPhysAddr + osc_buf_size * 3);
        if (m_maxChannels >= 3){
            setRegister(_OscMap,&(_OscMap->dma_dst_addr1_ch3),m_BufferPhysAddr + osc_buf_size * 4);
            setRegister(_OscMap,&(_OscMap->dma_dst_addr2_ch3),m_BufferPhysAddr + osc_buf_size * 5);
        }
        if (m_maxChannels >= 4){
            setRegister(_OscMap,&(_OscMap->dma_dst_addr1_ch4),m_BufferPhysAddr + osc_buf_size * 6);
            setRegister(_OscMap,&(_OscMap->dma_dst_addr2_ch4),m_BufferPhysAddr + osc_buf_size * 7);
        }

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

        // set bit mode 8 bit = 1 or 16 bit = 0
        setRegister(_OscMap,&(_OscMap->bitSwitch),m_is8BitMode ? 1 : 0);

        //_OscMap->trig_pre_samp = osc_buf_pre_samp;

        // Trigger post samples
        setRegister(_OscMap,&(_OscMap->trig_post_samp),osc_buf_post_samp);
        // _OscMap->trig_post_samp = osc_buf_post_samp;

        // Decimate factor
        setRegister(_OscMap,&(_OscMap->dec_factor),m_dec_factor);
        //_OscMap->dec_factor = m_dec_factor;

        setRegister(_OscMap,&(_OscMap->calib_offset_ch1),m_calib_offset_ch[0]);

        setRegister(_OscMap,&(_OscMap->calib_gain_ch1),m_calib_gain_ch[0]);

        setRegister(_OscMap,&(_OscMap->calib_offset_ch2),m_calib_offset_ch[1]);

        setRegister(_OscMap,&(_OscMap->calib_gain_ch2),m_calib_gain_ch[1]);


        setRegister(_OscMap,&(_OscMap->filt_coeff_aa_ch1),m_AA_ch[0]);
        setRegister(_OscMap,&(_OscMap->filt_coeff_bb_ch1),m_BB_ch[0]);
        setRegister(_OscMap,&(_OscMap->filt_coeff_kk_ch1),m_KK_ch[0]);
        setRegister(_OscMap,&(_OscMap->filt_coeff_pp_ch1),m_PP_ch[0]);

        setRegister(_OscMap,&(_OscMap->filt_coeff_aa_ch2),m_AA_ch[1]);
        setRegister(_OscMap,&(_OscMap->filt_coeff_bb_ch2),m_BB_ch[1]);
        setRegister(_OscMap,&(_OscMap->filt_coeff_kk_ch2),m_KK_ch[1]);
        setRegister(_OscMap,&(_OscMap->filt_coeff_pp_ch2),m_PP_ch[1]);

        if (m_maxChannels >= 3){
            setRegister(_OscMap,&(_OscMap->calib_offset_ch3),m_calib_offset_ch[2]);
            setRegister(_OscMap,&(_OscMap->calib_gain_ch3),m_calib_gain_ch[2]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_aa_ch3),m_AA_ch[2]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_bb_ch3),m_BB_ch[2]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_kk_ch3),m_KK_ch[2]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_pp_ch3),m_PP_ch[2]);
        }

        if (m_maxChannels >= 4){
            setRegister(_OscMap,&(_OscMap->calib_offset_ch4),m_calib_offset_ch[3]);
            setRegister(_OscMap,&(_OscMap->calib_gain_ch4),m_calib_gain_ch[3]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_aa_ch4),m_AA_ch[3]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_bb_ch4),m_BB_ch[3]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_kk_ch4),m_KK_ch[3]);
            setRegister(_OscMap,&(_OscMap->filt_coeff_pp_ch4),m_PP_ch[3]);
        }
        // printReg();
}


auto COscilloscope::setFilterCalibration(uint8_t ch,int32_t _aa,int32_t _bb, int32_t _kk, int32_t _pp) -> void {
    m_AA_ch[ch] = _aa;
    m_BB_ch[ch] = _bb;
    m_KK_ch[ch] = _kk;
    m_PP_ch[ch] = _pp;
}

auto COscilloscope::set8BitMode(bool mode) -> void{
    m_is8BitMode = mode;
}

auto COscilloscope::setFilterBypass(bool _state) -> void {
    m_filterBypass = _state;
}

auto COscilloscope::prepare() -> void {
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

auto COscilloscope::setCalibration(uint8_t ch,int32_t _offset,float _gain) -> void{
    if (_gain >= 2) _gain = 1.999999;
    if (_gain < 0)  _gain = 0;
    if (m_fpgaBits > 16){
        fprintf(stderr,"[Fatal Error] ADC must be lower or equal 16 bit. Now: %d\n",m_fpgaBits);
        m_fpgaBits = 16;
    }
    m_calib_offset_ch[ch] =  _offset * -( pow(2, 16 - m_fpgaBits));
    m_calib_gain_ch[ch] = _gain * 32768;
}

auto COscilloscope::next(uint8_t *&_buffer1,uint8_t *&_buffer2,uint8_t *&_buffer3,uint8_t *&_buffer4, size_t &_size,uint32_t &_overFlow) -> bool {
    _buffer1 = m_maxChannels > 0 ? m_OscBuffer[0] + osc_buf_size * m_OscBufferNumber : nullptr;
    _buffer2 = m_maxChannels > 1 ? m_OscBuffer[1] + osc_buf_size * m_OscBufferNumber : nullptr;
    _buffer3 = m_maxChannels > 2 ? m_OscBuffer[2] + osc_buf_size * m_OscBufferNumber : nullptr;
    _buffer4 = m_maxChannels > 3 ? m_OscBuffer[3] + osc_buf_size * m_OscBufferNumber : nullptr;

    // This fix for FPGA
    _overFlow = (m_OscBufferNumber == 1 ? m_OscMap->lost_samples_buf1_ch1 : m_OscMap->lost_samples_buf2_ch1);
    _size = osc_buf_size;
    return true;
}



auto COscilloscope::clearBuffer() -> bool{
    // clearInterrupt();

    uint32_t clearFlag = (m_OscBufferNumber == 0 ? 0x00000004 : 0x00000008); // reset buffer
    setRegister(m_OscMap,&(m_OscMap->dma_ctrl), clearFlag );
    m_OscBufferNumber = (m_OscBufferNumber == 0) ? 1 : 0;
    return true;
}

auto COscilloscope::wait() -> bool{
    int32_t cnt = 1;
    constexpr size_t cnt_size = sizeof(cnt);
    ssize_t bytes = write(m_Fd, &cnt, cnt_size); // Unmmask interrupt
    if (bytes == cnt_size) {
//      wait for the interrupt
//      printf("Wait Itr\n");
        struct pollfd pfd = {.fd = m_Fd, .events = POLLIN ,.revents = 0};
        int timeout_ms = 1000;
        int rv = poll(&pfd, 1, timeout_ms);
//      clear the interrupt
        if (rv >= 1) {
               uint32_t info;
               read(m_Fd, &info, sizeof(info));
               clearInterrupt();
//               printf("Itr\n");
        } else if (rv == 0) {
               return false;
        } else {
               perror("UIO::wait()");
        }

        return true;
    }
    return false;
}

auto COscilloscope::clearInterrupt() -> bool{
    std::lock_guard lock(m_waitLock);
   //  fprintf(stderr,"clearInterrupt()\n");
    setRegister(m_OscMap,&(m_OscMap->dma_ctrl), 0x00000002 );
    return true;
}

auto COscilloscope::stop() -> void {
    std::lock_guard lock(m_waitLock);
    if (m_OscMap != nullptr){
        setRegister(m_OscMap,&(m_OscMap->event_sts),UINT32_C(0x00000004));
    }else {
        std::cerr << "Error: COscilloscope::stop()" << std::endl;
        exit(-1);
    }
}

auto COscilloscope::getDecimation() -> u_int32_t {
    return m_dec_factor;
}

auto COscilloscope::getOSCRate() -> uint32_t{
    if (m_dec_factor == 0) return 0;
    return m_adcMaxSpeed / m_dec_factor;
}

auto COscilloscope::isMaster() -> BoardMode{
    if (m_OscMap != nullptr){
        usleep(100);
        BoardMode mode = BoardMode::UNKNOWN;
        if (m_OscMap->mode_slave_sts == 0x1){
            mode = BoardMode::MASTER;
        }

        if (m_OscMap->mode_slave_sts == 0x3){
            mode = BoardMode::SLAVE;
        }
        return mode;
    }else{
        return BoardMode::UNKNOWN;
    }
}

auto COscilloscope::printReg() -> void{
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
        fprintf(stderr,"0x5C lost_samples_buf1 = 0x%X\n", m_OscMap->lost_samples_buf1_ch1);
        fprintf(stderr,"0x60 lost_samples_buf2 = 0x%X\n", m_OscMap->lost_samples_buf2_ch1);
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
        fprintf(stderr,"0x100 mode_slave_sts = 0x%X\n", m_OscMap->mode_slave_sts);


        fprintf(stderr,"0x15C lost_samples_buf1_ch3 = 0x%X\n", m_OscMap->lost_samples_buf1_ch3);
        fprintf(stderr,"0x160 lost_samples_buf2_ch3 = 0x%X\n", m_OscMap->lost_samples_buf2_ch3);
        fprintf(stderr,"0x164 dma_dst_addr1_ch3 = 0x%X\n", m_OscMap->dma_dst_addr1_ch3);
        fprintf(stderr,"0x168 dma_dst_addr2_ch3 = 0x%X\n", m_OscMap->dma_dst_addr2_ch3);
        fprintf(stderr,"0x16C dma_dst_addr1_ch4 = 0x%X\n", m_OscMap->dma_dst_addr1_ch4);
        fprintf(stderr,"0x170 dma_dst_addr2_ch4 = 0x%X\n", m_OscMap->dma_dst_addr2_ch4);

        fprintf(stderr,"0x174 calib_offset_ch3 = 0x%X\n", m_OscMap->calib_offset_ch3);
        fprintf(stderr,"0x178 calib_gain_ch3 = 0x%X\n", m_OscMap->calib_gain_ch3);
        fprintf(stderr,"0x17C calib_offset_ch4 = 0x%X\n", m_OscMap->calib_offset_ch4);
        fprintf(stderr,"0x180 calib_gain_ch4 = 0x%X\n", m_OscMap->calib_gain_ch4);

        fprintf(stderr,"0x19C lost_samples_buf1_ch4 = 0x%X\n", m_OscMap->lost_samples_buf1_ch3);
        fprintf(stderr,"0x1A0 lost_samples_buf2_ch4 = 0x%X\n", m_OscMap->lost_samples_buf2_ch4);
        fprintf(stderr,"0x1A4 write_pointer_ch3 = 0x%X\n", m_OscMap->write_pointer_ch3);
        fprintf(stderr,"0x1A8 write_pointer_ch4 = 0x%X\n", m_OscMap->write_pointer_ch4);

        fprintf(stderr,"0x1C0 filt_coeff_aa_ch3 = 0x%X\n", m_OscMap->filt_coeff_aa_ch3);
        fprintf(stderr,"0x1C4 filt_coeff_bb_ch3 = 0x%X\n", m_OscMap->filt_coeff_bb_ch3);
        fprintf(stderr,"0x1C8 filt_coeff_kk_ch3 = 0x%X\n", m_OscMap->filt_coeff_kk_ch3);
        fprintf(stderr,"0x1CC filt_coeff_pp_ch3 = 0x%X\n", m_OscMap->filt_coeff_pp_ch3);
        fprintf(stderr,"0x1D0 filt_coeff_aa_ch4 = 0x%X\n", m_OscMap->filt_coeff_aa_ch4);
        fprintf(stderr,"0x1D4 filt_coeff_bb_ch4 = 0x%X\n", m_OscMap->filt_coeff_bb_ch4);
        fprintf(stderr,"0x1D8 filt_coeff_kk_ch4 = 0x%X\n", m_OscMap->filt_coeff_kk_ch4);
        fprintf(stderr,"0x1DC filt_coeff_pp_ch4 = 0x%X\n", m_OscMap->filt_coeff_pp_ch4);
}
