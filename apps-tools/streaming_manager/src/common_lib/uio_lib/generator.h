#ifndef UIO_LIB_GENERATOR_H
#define UIO_LIB_GENERATOR_H


#include <cstdint>
#include <memory>
#include <mutex>
#include "uio_parser.h"
#include "data_lib/neon_asm.h"

namespace uio_lib {

constexpr uint32_t gen0_event_id = 0x1;
constexpr uint32_t gen1_event_id = 0x2;
constexpr uint32_t dac_buf_size = (65536)/2;


struct GeneratorMapT
{
    uint32_t config;                // 0   - offset
    uint32_t chA_calib;             // 4   - offset
    uint32_t chA_counter_step;      // 8   - offset
    uint32_t chA_read_pointer;      // 12   - offset 0x0C
    uint32_t chB_calib;             // 16   - offset 0x10
    uint32_t chB_counter_step;      // 20   - offset 0x14
    uint32_t chB_read_pointer;      // 24   - offset 0x18
    uint32_t event_status;          // 28   - offset 0x1C
    uint32_t event_select;          // 32   - offset 0x20
    uint32_t trig_mask;             // 36   - offset 0x24
    uint32_t dma_control;           // 40   - offset 0x28
    uint32_t ch_dma_status;         // 44   - offset 0x2C
    uint32_t unused;                // 48   - offset 0x30
    uint32_t dma_size;              // 52   - offset 0x34
    uint32_t chA_dma_addr1;         // 56   - offset 0x38
    uint32_t chA_dma_addr2;         // 60   - offset 0x3C
    uint32_t chB_dma_addr1;         // 64   - offset 0x40
    uint32_t chB_dma_addr2;         // 68   - offset 0x44
};


class CGenerator
{
public:
    using Ptr = std::shared_ptr<CGenerator>;

    static Ptr create(const UioT &_uio, bool _channel1Enable, bool _channel2Enable,uint32_t dacHz,uint32_t maxDacHz);

    CGenerator(bool _channel1Enable,bool _channel2Enable, int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t dacHz,uint32_t maxDacHz);
    ~CGenerator();

    auto getDacHz() -> uint32_t;
    auto setDacHz(uint32_t hz) -> bool;
    auto prepare() -> void;
    auto initFirst(uint8_t *_buffer1,uint8_t *_buffer2, size_t _size_ch1, size_t _size_ch2) -> bool;
    auto initSecond(uint8_t *_buffer1,uint8_t *_buffer2, size_t _size_ch1, size_t _size_ch2) -> bool;
    
    auto write(uint8_t *_buffer1,uint8_t *_buffer2, size_t _size_ch1, size_t _size_ch2) -> bool;
    auto setCalibration(int32_t ch1_offset,float ch1_gain, int32_t ch2_offset, float ch2_gain) -> void;
    auto start() -> void;
    auto stop() -> void;
    auto printReg() -> void;

private:

    CGenerator(const CGenerator &) = delete;
    CGenerator(CGenerator &&) = delete;
    CGenerator& operator=(const CGenerator&) =delete;
    CGenerator& operator=(const CGenerator&&) =delete;

    auto setReg(volatile GeneratorMapT *_OscMap) -> void;

    bool         m_Channel1;
    bool         m_Channel2;
    int          m_Fd;
    void        *m_Regset;
    size_t       m_RegsetSize;
    void        *m_Buffer;
    size_t       m_BufferSize;
    uintptr_t    m_BufferPhysAddr;
    volatile     GeneratorMapT *m_Map;
    uint8_t     *m_Buffer1;
    uint8_t     *m_Buffer2;
    unsigned     m_BufferNumber[2];
    std::mutex   m_waitLock;
    int32_t      m_calib_offset_ch1;
    uint32_t     m_calib_gain_ch1;
    int32_t      m_calib_offset_ch2;
    uint32_t     m_calib_gain_ch2;
    uint32_t     m_maxDacSpeedHz;
    uint32_t     m_dacSpeedHz;
};

}
#endif
