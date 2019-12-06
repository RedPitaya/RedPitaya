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
    uint32_t event_sts;
    uint32_t event_sel;
    uint32_t trig_mask;
    uint32_t _reserved_0;
    uint32_t trig_pre_samp;
    uint32_t trig_post_samp;
    uint32_t trig_pre_cnt;
    uint32_t trig_post_cnt;
    uint32_t trig_low_level;
    uint32_t trig_high_level;
    uint32_t trig_edge;
    uint32_t _reserved_1;
    uint32_t dec_factor;
    uint32_t dec_rshift;
    uint32_t avg_en_addr;
    uint32_t filt_bypass;
    uint32_t filt_coeff_aa;
    uint32_t filt_coeff_bb;
    uint32_t filt_coeff_kk;
    uint32_t filt_coeff_pp;
    uint32_t dma_ctrl;
    uint32_t dma_dst_addr1;
    uint32_t dma_dst_addr2;
    uint32_t dma_buf_size;
    uint32_t calib_offset;
    uint32_t calib_gain;
};

class COscilloscope
{
public:
    using Ptr = std::shared_ptr<COscilloscope>;

    static Ptr Create(const UioT &_uio, unsigned _channel,uint32_t _dec_factor);

    COscilloscope(unsigned _channel, int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t _dec_factor);
    COscilloscope(const COscilloscope &) = delete;
    COscilloscope(COscilloscope &&) = delete;
    ~COscilloscope();

    void prepare();
    bool next(uint8_t *&_buffer, size_t &_size);
    void stop();

private:
    unsigned m_Channel;
    int m_Fd;
    void *m_Regset;
    size_t m_RegsetSize;
    void *m_Buffer;
    size_t m_BufferSize;
    uintptr_t m_BufferPhysAddr;
    volatile OscilloscopeMapT *m_OscMap;
    uint8_t *m_OscBuffer;
    unsigned m_OscBufferNumber;
    uint32_t m_dec_factor;
};
