#ifndef UIO_LIB_OSCILLOSCOPE_H
#define UIO_LIB_OSCILLOSCOPE_H

#include <cstdint>
#include <memory>
#include <mutex>
#include "uio_parser.h"

namespace uio_lib {

constexpr uint32_t osc0_event_id = 2;
constexpr uint32_t osc1_event_id = 3;
// constexpr uint32_t osc0_baseaddr = 0;
// constexpr uint32_t osc1_baseaddr = 256;

constexpr uint32_t osc_buf_size = (65536) / 2.0;
constexpr uint32_t osc_buf_pre_samp = osc_buf_size / 4;
constexpr uint32_t osc_buf_post_samp = (osc_buf_size / 4) * 3;

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
    uint32_t digitalLoopBack;       // 64  - offset 0x40
    uint32_t bitSwitch;             // 68  - offset 0x44
    uint32_t reserv[2];
    uint32_t dma_ctrl;              // 80  - offset 0x50
    uint32_t dma_sts_addr;          // 84  - offset 0x54
    uint32_t dma_buf_size;          // 88  - offset 0x58
    uint32_t lost_samples_buf1_ch1; // 92  - offset 0x5C
    uint32_t lost_samples_buf2_ch1; // 96  - offset 0x60
    uint32_t dma_dst_addr1_ch1;     // 100 - offset 0x64
    uint32_t dma_dst_addr2_ch1;     // 104 - offset 0x68
    uint32_t dma_dst_addr1_ch2;     // 108 - offset 0x6C
    uint32_t dma_dst_addr2_ch2;     // 112 - offset 0x70
    uint32_t calib_offset_ch1;      // 116 - offset 0x74
    uint32_t calib_gain_ch1;        // 120 - offset 0x78
    uint32_t calib_offset_ch2;      // 124 - offset 0x7C
    uint32_t calib_gain_ch2;        // 128 - offset 0x80
    uint32_t reserv2[6];
    uint32_t lost_samples_buf1_ch2;   // 156 - offset 0x9C
    uint32_t lost_samples_buf2_ch2;   // 160 - offset 0xA0
    uint32_t write_pointer_ch1;       // 164 - offset 0xA4
    uint32_t write_pointer_ch2;       // 168 - offset 0xA8
    uint32_t reserv3[5];
    uint32_t filt_coeff_aa_ch1;       // 192  - offset 0xC0
    uint32_t filt_coeff_bb_ch1;       // 196  - offset 0xC4
    uint32_t filt_coeff_kk_ch1;       // 200  - offset 0xC8
    uint32_t filt_coeff_pp_ch1;       // 204  - offset 0xCC
    uint32_t filt_coeff_aa_ch2;       // 208  - offset 0xD0
    uint32_t filt_coeff_bb_ch2;       // 212  - offset 0xD4
    uint32_t filt_coeff_kk_ch2;       // 216  - offset 0xD8
    uint32_t filt_coeff_pp_ch2;       // 220  - offset 0xDC
    uint32_t reserv4[8];
    uint32_t mode_slave_sts;          // 256  - offset 0x100

    // Extended regset for stream_app_4ch project
    uint32_t reserv5[22];
    uint32_t lost_samples_buf1_ch3;   // 348 - offset 0x15C
    uint32_t lost_samples_buf2_ch3;   // 352 - offset 0x160
    uint32_t dma_dst_addr1_ch3;       // 356 - offset 0x164
    uint32_t dma_dst_addr2_ch3;       // 360 - offset 0x168
    uint32_t dma_dst_addr1_ch4;       // 364 - offset 0x16C
    uint32_t dma_dst_addr2_ch4;       // 368 - offset 0x170
    uint32_t calib_offset_ch3;        // 372 - offset 0x174
    uint32_t calib_gain_ch3;          // 376 - offset 0x178
    uint32_t calib_offset_ch4;        // 380 - offset 0x17C
    uint32_t calib_gain_ch4;          // 384 - offset 0x180
    uint32_t reserv6[6];
    uint32_t lost_samples_buf1_ch4;   // 412 - offset 0x19C
    uint32_t lost_samples_buf2_ch4;   // 416 - offset 0x1A0
    uint32_t write_pointer_ch3;       // 420 - offset 0x1A4
    uint32_t write_pointer_ch4;       // 424 - offset 0x1A8
    uint32_t reserv7[5];
    uint32_t filt_coeff_aa_ch3;       // 448  - offset 0x1C0
    uint32_t filt_coeff_bb_ch3;       // 452  - offset 0x1C4
    uint32_t filt_coeff_kk_ch3;       // 456  - offset 0x1C8
    uint32_t filt_coeff_pp_ch3;       // 460  - offset 0x1CC
    uint32_t filt_coeff_aa_ch4;       // 464  - offset 0x1D0
    uint32_t filt_coeff_bb_ch4;       // 468  - offset 0x1D4
    uint32_t filt_coeff_kk_ch4;       // 472  - offset 0x1D8
    uint32_t filt_coeff_pp_ch4;       // 476  - offset 0x1DC


};

enum BoardMode {
    UNKNOWN,
    MASTER,
    SLAVE
};


class COscilloscope
{
public:
    using Ptr = std::shared_ptr<COscilloscope>;

    static Ptr create(const UioT &_uio, uint32_t _dec_factor,bool _isMaster,uint32_t _adcMaxSpeed,bool _isADCFilterPresent,uint8_t _fpgaBits,uint8_t _maxChannels);

    COscilloscope(int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t _dec_factor,bool _isMaster,uint32_t _adcMaxSpeed,bool _isADCFilterPresent,uint8_t _fpgaBits,uint8_t _maxChannels);
    ~COscilloscope();

    auto prepare() -> void;
    auto next(uint8_t *&_buffer1,uint8_t *&_buffer2,uint8_t *&_buffer3,uint8_t *&_buffer4, size_t &_size,uint32_t &_overFlow) -> bool;
    auto setCalibration(uint8_t ch,int32_t _offset,float _gain) -> void;
    auto setFilterCalibration(uint8_t ch,int32_t _aa,int32_t _bb, int32_t _kk, int32_t _pp) -> void;
    auto setFilterBypass(bool _state) -> void;
    auto set8BitMode(bool mode) -> void;
    auto clearBuffer() -> bool;
    auto wait() -> bool;
    auto clearInterrupt() -> bool;
    auto stop() -> void;
    auto printReg() -> void;
    auto getDecimation() -> uint32_t;
    auto getOSCRate() -> uint32_t;
    auto isMaster() -> BoardMode;

private:

    COscilloscope(const COscilloscope &) = delete;
    COscilloscope(COscilloscope &&) = delete;
    COscilloscope& operator=(const COscilloscope&) =delete;
    COscilloscope& operator=(const COscilloscope&&) =delete;

    auto setReg(volatile OscilloscopeMapT *_OscMap) -> void;

    int          m_Fd;
    void        *m_Regset;
    size_t       m_RegsetSize;
    void        *m_Buffer;
    size_t       m_BufferSize;
    uintptr_t    m_BufferPhysAddr;
    volatile     OscilloscopeMapT *m_OscMap;
    uint8_t     *m_OscBuffer[4];
    unsigned     m_OscBufferNumber;
    uint32_t     m_dec_factor;
    std::mutex   m_waitLock;
    int32_t      m_calib_offset_ch[4];
    uint32_t     m_calib_gain_ch[4];
    int32_t      m_AA_ch[4];
    int32_t      m_BB_ch[4];
    int32_t      m_PP_ch[4];
    int32_t      m_KK_ch[4];
    bool         m_filterBypass;
    bool         m_isMaster;
    bool         m_is8BitMode;
    uint32_t     m_adcMaxSpeed;
    bool         m_isADCFilterPresent;
    uint8_t      m_fpgaBits;
    uint8_t      m_maxChannels;
};

}

#endif
