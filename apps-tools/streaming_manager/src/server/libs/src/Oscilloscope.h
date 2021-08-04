#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <UioParser.h>
#include "log.h"

constexpr uint32_t osc0_event_id = 2;
constexpr uint32_t osc1_event_id = 3;
constexpr uint32_t osc0_baseaddr = 0;
constexpr uint32_t osc1_baseaddr = 256;

constexpr uint32_t osc_buf_size = (65536) / 2.0;
constexpr uint32_t osc_buf_pre_samp = osc_buf_size / 4;
constexpr uint32_t osc_buf_post_samp = (osc_buf_size / 4) * 3;

#ifdef Z20_250_12
struct OscilloscopeMapT
{
    uint32_t event_sts;             // 0   - offset
    uint32_t event_sel;             // 4   - offset
    uint32_t trig_mask;             // 8   - offset
    uint32_t _reserved_0;           // 12  - offset
    uint32_t trig_pre_samp;         // 16  - offset 0x10
    uint32_t trig_post_samp;        // 20  - offset 0x14
    uint32_t trig_pre_cnt;          // 24  - offset 0x18
    uint32_t trig_post_cnt;         // 28  - offset 0x1C
    uint32_t trig_low_level;        // 32  - offset 0x20
    uint32_t trig_high_level;       // 36  - offset 0x24
    uint32_t trig_edge;             // 40  - offset 0x28
    uint32_t _reserved_1;           // 44  - offset
    uint32_t dec_factor;            // 48  - offset 0x30
    uint32_t dec_rshift;            // 52  - offset 0x34
    uint32_t avg_en_addr;           // 56  - offset 0x38
    uint32_t filt_bypass;           // 60  - offset 0x3C
    uint32_t filt_coeff_aa;         // 64  - offset 0x40
    uint32_t filt_coeff_bb;         // 68  - offset 0x44
    uint32_t filt_coeff_kk;         // 72  - offset 0x48
    uint32_t filt_coeff_pp;         // 76  - offset 0x4C
    uint32_t dma_ctrl;              // 80  - offset 0x50
    uint32_t dma_sts_addr;          // 84  - offset 0x54
    uint32_t dma_buf_size;          // 88  - offset 0x58
    uint32_t lost_samples_buf1;     // 92  - offset 0x5C
    uint32_t lost_samples_buf2;     // 96  - offset 0x60
    uint32_t dma_dst_addr1_ch1;     // 100 - offset 0x64
    uint32_t dma_dst_addr2_ch1;     // 104 - offset 0x68
    uint32_t dma_dst_addr1_ch2;     // 108 - offset 0x6C
    uint32_t dma_dst_addr2_ch2;     // 112 - offset 0x70
    uint32_t calib_offset_ch1;      // 116 - offset 0x74
    uint32_t calib_gain_ch1;        // 120 - offset 0x78  
    uint32_t calib_offset_ch2;      // 124 - offset 0x7C
    uint32_t calib_gain_ch2;        // 128 - offset 0x80   
};
#else
struct OscilloscopeMapT
{
    uint32_t event_sts;             // 0   - offset
    uint32_t event_sel;             // 4   - offset
    uint32_t trig_mask;             // 8   - offset
    uint32_t _reserved_0;           // 12  - offset
    uint32_t trig_pre_samp;         // 16  - offset 0x10
    uint32_t trig_post_samp;        // 20  - offset 0x14
    uint32_t trig_pre_cnt;          // 24  - offset 0x18
    uint32_t trig_post_cnt;         // 28  - offset 0x1C
    uint32_t trig_low_level;        // 32  - offset 0x20
    uint32_t trig_high_level;       // 36  - offset 0x24
    uint32_t trig_edge;             // 40  - offset 0x28
    uint32_t _reserved_1;           // 44  - offset
    uint32_t dec_factor;            // 48  - offset 0x30
    uint32_t dec_rshift;            // 52  - offset 0x34
    uint32_t avg_en_addr;           // 56  - offset 0x38
    uint32_t filt_bypass;           // 60  - offset 0x3C
    uint32_t reserv[4];
    uint32_t dma_ctrl;              // 80  - offset 0x50
    uint32_t dma_sts_addr;          // 84  - offset 0x54
    uint32_t dma_buf_size;          // 88  - offset 0x58
    uint32_t lost_samples_buf1;     // 92  - offset 0x5C
    uint32_t lost_samples_buf2;     // 96  - offset 0x60
    uint32_t dma_dst_addr1_ch1;     // 100 - offset 0x64
    uint32_t dma_dst_addr2_ch1;     // 104 - offset 0x68
    uint32_t dma_dst_addr1_ch2;     // 108 - offset 0x6C
    uint32_t dma_dst_addr2_ch2;     // 112 - offset 0x70
    uint32_t calib_offset_ch1;      // 116 - offset 0x74
    uint32_t calib_gain_ch1;        // 120 - offset 0x78  
    uint32_t calib_offset_ch2;      // 124 - offset 0x7C
    uint32_t calib_gain_ch2;        // 128 - offset 0x80
    uint32_t reserv2[6];
    uint32_t lost_samples_buf1_ch2;  // 156 - offset 0x9C
    uint32_t lost_samples_buf2_ch2;  // 160 - offset 0xA0
    uint32_t write_pointer_ch1;     // 164 - offset 0xA4
    uint32_t write_pointer_ch2;     // 168 - offset 0xA8
    uint32_t reserv3[5];
    uint32_t filt_coeff_aa_ch1;       // 192  - offset 0xC0
    uint32_t filt_coeff_bb_ch1;       // 196  - offset 0xC4
    uint32_t filt_coeff_kk_ch1;       // 200  - offset 0xC8
    uint32_t filt_coeff_pp_ch1;       // 204  - offset 0xCC
    uint32_t filt_coeff_aa_ch2;       // 208  - offset 0xD0
    uint32_t filt_coeff_bb_ch2;       // 212  - offset 0xD4
    uint32_t filt_coeff_kk_ch2;       // 216  - offset 0xD8
    uint32_t filt_coeff_pp_ch2;       // 220  - offset 0xDC
};
#endif

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
    bool next(uint8_t *&_buffer1,uint8_t *&_buffer2, size_t &_size,uint32_t &_overFlow);
    void setCalibration(int32_t ch1_offset,float ch1_gain, int32_t ch2_offset, float ch2_gain);
    void setFilterCalibrationCh1(int32_t _aa,int32_t _bb, int32_t _pp, int32_t _kk);
    void setFilterCalibrationCh2(int32_t _aa,int32_t _bb, int32_t _pp, int32_t _kk);
    void setFilterBypass(bool _state);
    bool clearBuffer();
    bool wait();
    bool clearInterrupt();
    void stop();

private:
    void setReg(volatile OscilloscopeMapT *_OscMap);

    bool         m_Channel1;
    bool         m_Channel2;
    int          m_Fd;
    void        *m_Regset;
    size_t       m_RegsetSize;
    void        *m_Buffer;
    size_t       m_BufferSize;
    uintptr_t    m_BufferPhysAddr;
    volatile     OscilloscopeMapT *m_OscMap;
    uint8_t     *m_OscBuffer1;
    uint8_t     *m_OscBuffer2;
    unsigned     m_OscBufferNumber;
    uint32_t     m_dec_factor;
    std::mutex   m_waitLock;
    int32_t      m_calib_offset_ch1;
    uint32_t     m_calib_gain_ch1;
    int32_t      m_calib_offset_ch2;
    uint32_t     m_calib_gain_ch2;
    int32_t      m_AA_ch1;
    int32_t      m_BB_ch1;
    int32_t      m_PP_ch1;
    int32_t      m_KK_ch1;
    int32_t      m_AA_ch2;
    int32_t      m_BB_ch2;
    int32_t      m_PP_ch2;
    int32_t      m_KK_ch2;
    bool         m_filterBypass;
};
