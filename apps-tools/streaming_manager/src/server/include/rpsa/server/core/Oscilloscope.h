#pragma once

#include <cstdint>
#include <memory>

#include <UioParser.h>

constexpr uint32_t osc0_event_id = 2;
constexpr uint32_t osc1_event_id = 3;
constexpr uint32_t osc0_baseaddr = 0;
constexpr uint32_t osc1_baseaddr = 256;

constexpr uint32_t osc_buf_size = 65536;
constexpr uint32_t osc_buf_pre_samp = osc_buf_size / 4;
constexpr uint32_t osc_buf_post_samp = (osc_buf_size / 4) * 3;

struct OscilloscopeMapT
{
    uint32_t event_sts;             // 0 - offset
    uint32_t event_sel;             // 4 - offset
    uint32_t trig_mask;             // 8 - offset
    uint32_t _reserved_0;           // 12 - offset
    uint32_t trig_pre_samp;         // 16 - offset
    uint32_t trig_post_samp;        // 20 - offset
    uint32_t trig_pre_cnt;          // 24 - offset
    uint32_t trig_post_cnt;         // 28 - offset
    uint32_t trig_low_level;        // 32 - offset
    uint32_t trig_high_level;       // 36 - offset
    uint32_t trig_edge;             // 40 - offset
    uint32_t _reserved_1;           // 44 - offset
    uint32_t dec_factor;            // 48 - offset
    uint32_t dec_rshift;            // 52 - offset
    uint32_t avg_en_addr;           // 56 - offset
    uint32_t filt_bypass;           // 60 - offset
    uint32_t filt_coeff_aa;         // 64 - offset
    uint32_t filt_coeff_bb;         // 68 - offset
    uint32_t filt_coeff_kk;         // 72 - offset
    uint32_t filt_coeff_pp;         // 76 - offset
    uint32_t dma_ctrl;              // 80 - offset
    uint32_t dma_sts_addr;          // 84 - offset
    uint32_t dma_dst_addr1;         // 88 - offset
    uint32_t dma_dst_addr2;         // 92 - offset
    uint32_t dma_buf_size;          // 96 - offset
    uint32_t calib_offset;          // 100 - offset
    uint32_t calib_gain;            // 104 - offset
};

class COscilloscope
{
public:
    using Ptr = std::shared_ptr<COscilloscope>;

    static Ptr Create(const UioT &_uio, bool _channel1Enable, bool _channel2Enable, uint32_t _dec_factor);

    COscilloscope(bool _channel1Enable,bool _channel2Enable, int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t _dec_factor);
    COscilloscope(const COscilloscope &) = delete;
    COscilloscope(COscilloscope &&) = delete;
    ~COscilloscope();

    void prepare();
    bool next(uint8_t *&_buffer1,uint8_t *&_buffer2, size_t &_size,bool &_overFlow1 , bool &_overFlow2);
    bool changeBuffers();
    void stop();

private:
    void setReg(volatile OscilloscopeMapT *_OscMap ,unsigned int _Channel);

    bool m_Channel1;
    bool m_Channel2;
    int m_Fd;
    void *m_Regset;
    size_t m_RegsetSize;
    void *m_Buffer;
    size_t m_BufferSize;
    uintptr_t m_BufferPhysAddr;
    volatile OscilloscopeMapT *m_OscMap1;
    volatile OscilloscopeMapT *m_OscMap2;
    uint8_t *m_OscBuffer1;
    uint8_t *m_OscBuffer2;
    unsigned m_OscBufferNumber;
    uint32_t m_dec_factor;
};
