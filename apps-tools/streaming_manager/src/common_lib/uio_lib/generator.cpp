#include "generator.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include "logger_lib/file_logger.h"

using namespace uio_lib;

void* MmapNumberGen(int _fd, size_t _size, size_t _number) {
    const size_t offset = _number * getpagesize();
    return mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, offset);
}

inline void setRegister(__attribute__((unused)) volatile GeneratorMapT* baseOsc_addr, volatile uint32_t* reg, int32_t value, __attribute__((unused)) const char* info = nullptr) {
    // acprintf(stderr, PColor::RED, "\tSet register 0x%X <- 0x%X %s\n", (uint32_t)reg - (uint32_t)baseOsc_addr, value, info ? info : "");
    *reg = value;
}

#define XSTR(s) STR(s)
#define STR(s) #s
#define setRegisterVal(X, Y, Z)                                                    \
    {                                                                              \
        /* acprintf(stderr, PColor::RED, "\tSet %s <- 0x%X %s\n",XSTR(X),Y, Z); */ \
        X = Y;                                                                     \
    }

CGenerator::Ptr CGenerator::create(const UioT& _uio, bool _channel1Enable, bool _channel2Enable, uint32_t dacHz, uint32_t maxDacHz) {
    // Validation
    if (_uio.mapList.size() < 2) {
        // Error: validation.
        ERROR_LOG("Error: UIO validation.")
        return CGenerator::Ptr();
    }

    // Open file
    std::string path("/dev/" + _uio.name);
    int fd = open(path.c_str(), O_RDWR);

    if (fd == -1) {
        // Error: open file.
        ERROR_LOG("Error: open file.");
        return CGenerator::Ptr();
    }

    // Map
    void* regset = MmapNumberGen(fd, _uio.mapList[0].size, 0);

    if (regset == MAP_FAILED) {
        // Error: mmap
        ERROR_LOG("Error: mmap regset.");
        close(fd);
        return CGenerator::Ptr();
    }

    return std::make_shared<CGenerator>(_channel1Enable, _channel2Enable, fd, regset, _uio.mapList[0].size, dacHz, maxDacHz);
}

CGenerator::CGenerator(bool _channel1Enable, bool _channel2Enable, int _fd, void* _regset, size_t _regsetSize, uint32_t dacHz, uint32_t maxDacHz)
    : m_Channel1(_channel1Enable),
      m_Channel2(_channel2Enable),
      m_Fd(_fd),
      m_Regset(_regset),
      m_RegsetSize(_regsetSize),
      m_BufferSize(0),
      m_Map(nullptr),
      m_waitLock(),
      m_maxDacSpeedHz(maxDacHz),
      m_dacSpeedHz(dacHz) {
    // m_BufferNumber[0] = m_BufferNumber[1] = 0;
    m_calib_offset_ch1 = 0;
    m_calib_gain_ch1 = 0x2000;
    m_calib_offset_ch2 = 0;
    m_calib_gain_ch2 = 0x2000;
    uintptr_t Map = reinterpret_cast<uintptr_t>(m_Regset);
    m_Map = reinterpret_cast<GeneratorMapT*>(Map);
}

CGenerator::~CGenerator() {
    stop();
    munmap(m_Regset, m_RegsetSize);
    close(m_Fd);
}

auto CGenerator::getDacHz() -> uint32_t {
    return m_dacSpeedHz;
}

auto CGenerator::setDacHz(uint32_t hz) -> bool {
    if ((hz <= m_maxDacSpeedHz) && (((double)hz / (double)m_maxDacSpeedHz) * (1 << 16) < 1))
        return false;
    m_dacSpeedHz = hz;
    double coff = (double)m_dacSpeedHz / (double)m_maxDacSpeedHz;
    uint32_t step = (1 << 16) * coff;
    if (step == 0) {
        return false;
    }
    return true;
}

void CGenerator::setReg(volatile GeneratorMapT* _Map) {
    // Reset EVENT
    setRegister(_Map, &(_Map->event_status), 0);
    // Buffer
    setRegister(_Map, &(_Map->dma_size), m_BufferSize, "Buffer size");
    setRegister(_Map, &(_Map->chA_dma_addr1), 0, "Address chA buff 1");
    setRegister(_Map, &(_Map->chA_dma_addr2), 0, "Address chA buff 2");
    setRegister(_Map, &(_Map->chB_dma_addr1), 0, "Address chB buff 1");
    setRegister(_Map, &(_Map->chB_dma_addr2), 0, "Address chB buff 2");

    // Select event
    setRegister(_Map, &(_Map->event_select), gen0_event_id);

    // Event trig
    setRegister(_Map, &(_Map->trig_mask), 1);

    // Set trigger immediatly
    setRegisterVal(_Map->config.trig_chA, 0x1, "Set trig immediately chA");
    setRegisterVal(_Map->config.trig_chB, 0x1, "Set trig immediately chB");

    // Set calib for chA
    setRegister(_Map, &(_Map->chA_calib), m_calib_offset_ch1 << 16 | m_calib_gain_ch1, "Set calib for chA");

    // Set calib for chB
    setRegister(_Map, &(_Map->chB_calib), m_calib_offset_ch2 << 16 | m_calib_gain_ch2, "Set calib for chB");

    // Set step for pointer
    double coff = (double)m_dacSpeedHz / (double)m_maxDacSpeedHz;
    uint32_t step = (1 << 16) * coff;
    setRegister(_Map, &(_Map->chA_counter_step), step, "Set step for pointer chA");
    setRegister(_Map, &(_Map->chB_counter_step), step, "Set step for pointer chB");

    // Set streaming DMA, reset Buffers and flags
    setRegister(_Map, &(_Map->dma_control), 0x2222, "Set streaming DMA, reset Buffers and flags");
}

auto CGenerator::prepare() -> void {
    stop();

    if (m_Map != nullptr) {
        setReg(m_Map);
    } else {
        FATAL("Error: CGenerator::prepare()  can't init first channel")
    }
}

auto CGenerator::setCalibration(int32_t ch1_offset, float ch1_gain, int32_t ch2_offset, float ch2_gain) -> void {
    if (ch1_gain >= 2)
        ch1_gain = 1.999999;
    if (ch1_gain < 0)
        ch1_gain = 0;
    if (ch2_gain >= 2)
        ch2_gain = 1.999999;
    if (ch2_gain < 0)
        ch2_gain = 0;

    m_calib_offset_ch1 = ch1_offset;
    m_calib_offset_ch2 = ch2_offset;

    m_calib_gain_ch1 = ch1_gain * 0x2000;
    m_calib_gain_ch2 = ch2_gain * 0x2000;
}

auto CGenerator::setDataAddress(uint8_t index, uint32_t ch1, uint32_t ch2, uint32_t size) -> bool {
    bool ret = false;
    const std::lock_guard lock(m_waitLock);
    uint32_t status = m_Map->ch_dma_status;
    GenDMAStatus_t* sts = (GenDMAStatus_t*)&status;

    uint32_t chBuf1Wait[2] = {sts->read_done_buff1_chA, sts->read_done_buff1_chB};
    uint32_t chBuf2Wait[2] = {sts->read_done_buff2_chA, sts->read_done_buff2_chB};

    bool b1Wait = false;
    if (ch1 && ch2) {
        b1Wait = chBuf1Wait[0] && chBuf1Wait[1];
    } else if (ch1) {
        b1Wait = chBuf1Wait[0];
    } else if (ch2) {
        b1Wait = chBuf1Wait[1];
    }

    bool b2Wait = false;
    if (ch1 && ch2) {
        b2Wait = chBuf2Wait[0] && chBuf2Wait[1];
    } else if (ch1) {
        b2Wait = chBuf2Wait[0];
    } else if (ch2) {
        b2Wait = chBuf2Wait[1];
    }

    if (index == 0) {
        if (b1Wait) {
            // WARNING("status B1 0x%X",status)
            if (chBuf1Wait[0] && ch1 != 0)
                setRegister(m_Map, &(m_Map->chA_dma_addr1), ch1, "Address chA buff 1");
            if (chBuf1Wait[1] && ch2 != 0)
                setRegister(m_Map, &(m_Map->chB_dma_addr1), ch2, "Address chB buff 1");
            setRegister(m_Map, &(m_Map->dma_size), size, "Buffer size");
            int command = (ch1 != 0 ? 1 << 6 : 0) | (ch2 != 0 ? 1 << 14 : 0);
            setRegister(m_Map, &(m_Map->dma_control), command, "Reset index 1");
            ret = true;
        }
    }

    if (index == 1) {
        if (b2Wait) {
            // WARNING("status B2 0x%X",status)
            if (chBuf2Wait[0] && ch1 != 0)
                setRegister(m_Map, &(m_Map->chA_dma_addr2), ch1, "Address chA buff 2");
            if (chBuf2Wait[1] && ch2 != 0)
                setRegister(m_Map, &(m_Map->chB_dma_addr2), ch2, "Address chB buff 2");
            setRegister(m_Map, &(m_Map->dma_size), size, "Buffer size");
            int command = (ch1 != 0 ? 1 << 7 : 0) | (ch2 != 0 ? 1 << 15 : 0);
            setRegister(m_Map, &(m_Map->dma_control), command, "Reset index 2");
            ret = true;
        }
    }
    // if (!ret) WARNING("status 0x%X index %d",status,index)
    return ret;
}

auto CGenerator::setDataBits(bool is8BitCh1, bool is8BitCh2) -> void {
    setRegisterVal(m_Map->config.use8Bit_chA, is8BitCh1 ? 0x1 : 0, "Set trig immediately chA");
    setRegisterVal(m_Map->config.use8Bit_chB, is8BitCh2 ? 0x1 : 0, "Set trig immediately chB");
}

auto CGenerator::setDataSize(uint32_t size) -> void {
    m_BufferSize = size;
}

auto CGenerator::start(bool enableCh1, bool enableCh2) -> void {
    const std::lock_guard lock(m_waitLock);
    if (m_Map != nullptr) {
        setRegister(m_Map, &(m_Map->event_status), 0x2, "Start event");
        setRegister(m_Map, &(m_Map->dma_control), (enableCh1 ? 0x1 : 0) | (enableCh2 ? 0x100 : 0), "Start DMA");
    } else {
        FATAL("Memory map not initialized")
    }
}

auto CGenerator::stop() -> void {
    const std::lock_guard lock(m_waitLock);
    if (m_Map != nullptr) {
        // Stop EVENT
        setRegister(m_Map, &(m_Map->event_status), 0x4);
        // Stop DMA
        setRegister(m_Map, &(m_Map->dma_control), 0);
    } else {
        FATAL("Memory map not initialized")
    }
}

auto CGenerator::printReg() -> void {
    uint32_t* config = (uint32_t*)&(m_Map->config);
    fprintf(stderr, "printReg\n");
    fprintf(stderr, "0x00 Config = 0x%X\n", *config);
    fprintf(stderr, "0x04 chA_calib = 0x%X\n", m_Map->chA_calib);
    fprintf(stderr, "0x08 chA_counter_step = 0x%X\n", m_Map->chA_counter_step);
    fprintf(stderr, "0x0C chA_read_pointer = 0x%X\n", m_Map->chA_read_pointer);
    fprintf(stderr, "0x10 chB_calib = 0x%X\n", m_Map->chB_calib);
    fprintf(stderr, "0x14 chB_counter_step = 0x%X\n", m_Map->chB_counter_step);
    fprintf(stderr, "0x18 chB_read_pointer = 0x%X\n", m_Map->chB_read_pointer);
    fprintf(stderr, "0x1C event_status = 0x%X\n", m_Map->event_status);
    fprintf(stderr, "0x20 event_select = 0x%X\n", m_Map->event_select);
    fprintf(stderr, "0x24 trig_mask = 0x%X\n", m_Map->trig_mask);
    fprintf(stderr, "0x28 dma_control = 0x%X\n", m_Map->dma_control);
    fprintf(stderr, "0x2C ch_dma_status = 0x%X\n", m_Map->ch_dma_status);
    fprintf(stderr, "0x34 dma_size = 0x%X\n", m_Map->dma_size);
    fprintf(stderr, "0x38 chA_dma_addr1 = 0x%X\n", m_Map->chA_dma_addr1);
    fprintf(stderr, "0x3C chA_dma_addr2 = 0x%X\n", m_Map->chA_dma_addr2);
    fprintf(stderr, "0x40 chB_dma_addr1 = 0x%X\n", m_Map->chB_dma_addr1);
    fprintf(stderr, "0x44 chB_dma_addr2 = 0x%X\n", m_Map->chB_dma_addr2);
}
