#ifndef UIO_LIB_GENERATOR_H
#define UIO_LIB_GENERATOR_H

#include <cstdint>
#include <memory>
#include <mutex>
#include "trigger.h"
#include "uio_parser.h"

namespace uio_lib {

constexpr uint32_t gen0_event_id = 0x1;
constexpr uint32_t gen1_event_id = 0x2;
// constexpr uint32_t dac_buf_size = (65536)/2;

typedef struct GenConfig {
    uint32_t trig_chA : 4;
    uint32_t : 2;
    uint32_t use8Bit_chA : 1;
    uint32_t output0_chA : 1;
    uint32_t : 8;
    uint32_t trig_chB : 4;
    uint32_t : 2;
    uint32_t use8Bit_chB : 1;
    uint32_t output0_chB : 1;
    uint32_t : 8;
} GenConfig_t;

typedef struct GenDMAStatus {
    uint32_t wait_FIFO_buff1_chA : 1;  //  Wait for receive FIFO to empty out buffer 1
    uint32_t wait_Fill_buff1_chA : 1;  //  Wait for buffer 1 to fill up
    uint32_t wait_FIFO_buff2_chA : 1;  //  Wait for receive FIFO to empty out buffer 2
    uint32_t wait_Fill_buff2_chA : 1;  //  Wait for buffer 2 to fill up

    uint32_t reset_chA : 1;              //  Reset state
    uint32_t active_read_buff1_chA : 1;  //  Active read of buffer 1 state
    uint32_t active_read_buff2_chA : 1;  //  Active read of buffer 2 state
    uint32_t read_done_buff1_chA : 1;    //  Buffer 1 may be written to

    uint32_t read_done_buff2_chA : 1;  //  Buffer 2 may be written to
    uint32_t : 7;

    uint32_t wait_FIFO_buff1_chB : 1;  //  Wait for receive FIFO to empty out buffer 1
    uint32_t wait_Fill_buff1_chB : 1;  //  Wait for buffer 1 to fill up
    uint32_t wait_FIFO_buff2_chB : 1;  //  Wait for receive FIFO to empty out buffer 2
    uint32_t wait_Fill_buff2_chB : 1;  //  Wait for buffer 2 to fill up

    uint32_t reset_chB : 1;              //  Reset state
    uint32_t active_read_buff1_chB : 1;  //  Active read of buffer 1 state
    uint32_t active_read_buff2_chB : 1;  //  Active read of buffer 2 state
    uint32_t read_done_buff1_chB : 1;    //  Buffer 1 may be written to

    uint32_t read_done_buff2_chB : 1;  //  Buffer 2 may be written to
    uint32_t : 7;
} GenDMAStatus_t;

struct GeneratorMapT {
    GenConfig_t config;         // 0   - offset
    uint32_t chA_calib;         // 4   - offset
    uint32_t chA_counter_step;  // 8   - offset
    uint32_t chA_read_pointer;  // 12   - offset 0x0C
    uint32_t chB_calib;         // 16   - offset 0x10
    uint32_t chB_counter_step;  // 20   - offset 0x14
    uint32_t chB_read_pointer;  // 24   - offset 0x18
    uint32_t event_status;      // 28   - offset 0x1C
    uint32_t event_select;      // 32   - offset 0x20
    uint32_t trig_mask;         // 36   - offset 0x24
    uint32_t dma_control;       // 40   - offset 0x28
    uint32_t ch_dma_status;     // 44   - offset 0x2C
    uint32_t unused;            // 48   - offset 0x30
    uint32_t dma_size;          // 52   - offset 0x34
    uint32_t chA_dma_addr1;     // 56   - offset 0x38
    uint32_t chA_dma_addr2;     // 60   - offset 0x3C
    uint32_t chB_dma_addr1;     // 64   - offset 0x40
    uint32_t chB_dma_addr2;     // 68   - offset 0x44
    uint32_t setdec_chA;        // 72   - offset 0x48
    uint32_t setdec_chB;        // 76   - offset 0x4C
    uint32_t errs_rst;          // 80   - offset 0x50
    uint32_t errs_cnt_chA;      // 84   - offset 0x54
    uint32_t errs_cnt_chB;      // 88   - offset 0x58
    uint32_t loopback_en;       // 92   - offset 0x5C
    uint32_t out_shift_chA;     // 96   - offset 0x60
    uint32_t out_shift_chB;     // 100  - offset 0x64
    uint32_t trig_mode;         // 104  - offset 0x68: bits 1:0 trigger mode A/B, bits 9:8 armed A/B (read only)
    uint32_t reserv;            // 108  - offset 0x6C
    uint32_t diag_reg1;  // 112  - offset 0x70
    uint32_t diag_reg2;  // 114  - offset 0x74
    uint32_t diag_reg3;  // 118  - offset 0x78
    uint32_t diag_reg4;  // 122  - offset 0x7C
};

class CGenerator {
   public:
    using Ptr = std::shared_ptr<CGenerator>;

    static Ptr create(const UioT& _uio, bool _channel1Enable, bool _channel2Enable, uint32_t dacHz, uint32_t maxDacHz);

    CGenerator(bool _channel1Enable, bool _channel2Enable, int _fd, void* _regset, size_t _regsetSize, uint32_t dacHz, uint32_t maxDacHz);
    ~CGenerator();

    auto getDacHz() -> uint32_t;
    auto setDacHz(uint32_t hz) -> bool;

    auto prepare() -> void;

    auto setDataAddress(uint8_t index, uint32_t ch1, uint32_t ch2, uint32_t size) -> bool;
    auto setDataSize(uint32_t size) -> void;
    auto setDataBits(bool is8BitCh1, bool is8BitCh2) -> void;
    auto setCalibration(int32_t ch1_offset, float ch1_gain, int32_t ch2_offset, float ch2_gain) -> void;
    auto start(bool enableCh1, bool enableCh2) -> void;
    auto stop() -> void;
    auto printReg() -> void;

    // Trigger mode per channel: the channel fills its FIFO, arms and starts on the
    // next trigger edge. The setters apply on the next prepare() and fail while running.
    auto setTriggerMode(bool chA, bool chB) -> bool;
    auto getArmed() -> uint8_t;  // bit 0 - channel A, bit 1 - channel B
    auto setTriggerSources(uint32_t mask) -> bool;
    auto getTriggerSources() -> uint32_t;
    auto setEventSource(uint32_t source) -> bool;
    auto softwareTrigger() -> void;
    auto isRunning() -> bool;

   private:
    CGenerator(const CGenerator&) = delete;
    CGenerator(CGenerator&&) = delete;
    CGenerator& operator=(const CGenerator&) = delete;
    CGenerator& operator=(const CGenerator&&) = delete;

    auto setReg(volatile GeneratorMapT* _OscMap) -> void;

    bool m_Channel1;
    bool m_Channel2;
    int m_Fd;
    void* m_Regset;
    size_t m_RegsetSize;
    size_t m_BufferSize;
    uintptr_t m_BufferPhysAddr;
    volatile GeneratorMapT* m_Map;
    uint8_t* m_Buffer1;
    uint8_t* m_Buffer2;
    std::mutex m_waitLock;
    int32_t m_calib_offset_ch1;
    uint32_t m_calib_gain_ch1;
    int32_t m_calib_offset_ch2;
    uint32_t m_calib_gain_ch2;
    uint32_t m_maxDacSpeedHz;
    uint32_t m_dacSpeedHz;
    bool m_is8Bit[2];
    bool m_running;
    bool m_trigMode[2];
    uint32_t m_trigMask;
    uint32_t m_eventSource;
};

}  // namespace uio_lib
#endif
