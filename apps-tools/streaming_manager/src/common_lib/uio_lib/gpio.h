#ifndef UIO_LIB_GPIO_H
#define UIO_LIB_GPIO_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>
#include "trigger.h"
#include "uio_parser.h"

namespace uio_lib {

constexpr uint32_t gpio_event_id = EVENT_SRC_LA;

// Event register 0x88 drives la_event_op directly, in the rp_concat order (unlike the scope and DAC).
constexpr uint32_t GPIO_EVENT_TRIG = 1 << 0;
constexpr uint32_t GPIO_EVENT_STOP = 1 << 1;
constexpr uint32_t GPIO_EVENT_START = 1 << 2;
constexpr uint32_t GPIO_EVENT_RESET = 1 << 3;

// The register decoder uses address bits 7:0, the block repeats every 0x100 bytes.
struct GpioMapT {
    uint32_t acq_sts;       // 0x00 bit 1 trigger seen, bit 2 acquiring, bit 3 stopped
    uint32_t acq_cfg;       // 0x04 bit 0 continuous, bit 1 automatic (not connected in stream_app)
    uint32_t trig_mode;     // 0x08 bit 0 input, bit 1 output trigger mode; bits 8/9 armed (read only)
    uint32_t trig_blocks;   // 0x0C input block mode: buffers per trigger (0: no end)
    uint32_t cfg_pre;       // 0x10
    uint32_t cfg_post;      // 0x14
    uint32_t sts_pre;       // 0x18
    uint32_t sts_post;      // 0x1C
    uint32_t acq_time_lo;   // 0x20
    uint32_t acq_time_hi;   // 0x24
    uint32_t trig_time_lo;  // 0x28
    uint32_t trig_time_hi;  // 0x2C
    uint32_t stop_time_lo;  // 0x30
    uint32_t stop_time_hi;  // 0x34
    uint32_t reserv1[2];    // 0x38
    uint32_t cmp_mask;      // 0x40 pattern trigger: compared bits (7:0 P, 15:8 N)
    uint32_t cmp_value;     // 0x44 pattern trigger: value of the compared bits
    uint32_t edge_pos;      // 0x48 rising edge trigger bits
    uint32_t edge_neg;      // 0x4C falling edge trigger bits
    uint32_t decimation;    // 0x50 decimation - 1 (0: every sample)
    uint32_t rle;           // 0x54
    uint32_t cnt_current;   // 0x58
    uint32_t cnt_last;      // 0x5C
    uint32_t polarity;      // 0x60 inverted input lines (7:0 P, 15:8 N)
    uint32_t reserv2;       // 0x64
    uint32_t ext_trig;      // 0x68 see ext_trigger.h
    uint32_t reserv3;       // 0x6C
    uint32_t dir_p;         // 0x70 1 - input (high impedance), 0 - output
    uint32_t dir_n;         // 0x74
    uint32_t reserv4[2];    // 0x78
    uint32_t event_sel;     // 0x80
    uint32_t trig_mask;     // 0x84
    uint32_t event;         // 0x88
    uint32_t dma_ctrl_in;   // 0x8C
    uint32_t dma_ctrl_out;  // 0x90
    uint32_t dma_sts_in;    // 0x94
    uint32_t dma_sts_out;   // 0x98
    uint32_t dma_size;      // 0x9C shared by the input and the output
    uint32_t buf1_in;       // 0xA0
    uint32_t buf1_out;      // 0xA4
    uint32_t buf2_in;       // 0xA8
    uint32_t buf2_out;      // 0xAC
    uint32_t lost_buf1;     // 0xB0
    uint32_t lost_buf2;     // 0xB4
    uint32_t wp_in;         // 0xB8
    uint32_t rp_out;        // 0xBC
    uint32_t out_step;      // 0xC0
    uint32_t in_dma_diag;   // 0xC4 input DMA: {awlen, intr, fifo_dis, buffer, ovr2, ovr1, full2, full1, awready, wready, awvalid, wvalid, state}
    uint32_t axi_aw_b;      // 0xC8 write address handshakes << 16 | write responses (16 bit each, free running)
    uint32_t axi_w;         // 0xCC write data beats
    uint32_t axi_ar_rlast;  // 0xD0 read address handshakes << 16 | read bursts completed
    uint32_t axi_r;         // 0xD4 read data beats
    uint32_t in_trig_count; // 0xD8 triggers taken by the input since the start (series started or extended)
    uint32_t reserv5;       // 0xDC
    uint32_t in_lost[2][2]; // 0xE0 64 bit samples lost right before input buffer 1 {lo, hi}, 0xE8 buffer 2
};

// GPIO streaming block of stream_app (DIO0..7 P and N), present on the boards with rp_gpio
// in their stream_app (not on the 250-12 and the low latency boards).
//
// Input samples are 32 bit: bits 15:0 the P lines, bits 31:16 the N lines; each half is
// {RLE count (repeats - 1, 0 without RLE), 8 line bits}.
// Output samples are 32 bit: bits 31:24 the P lines, bits 23:16 the N lines, bits 7:0 the RLE count.
class CGpio {
   public:
    using Ptr = std::shared_ptr<CGpio>;

    static auto create(const std::vector<UioT>& uioList) -> Ptr;

    CGpio(int _fd, void* _regset, size_t _regsetSize);
    ~CGpio();

    // Configuration is written at once and refused while the DMA runs.
    auto setOutputs(uint8_t p, uint8_t n) -> bool;  // 1 - the line is driven by the output stream
    auto setPolarity(uint16_t mask) -> bool;  // inverted input lines: bits 7:0 P, 15:8 N
    auto setDecimation(uint32_t dec) -> bool;
    auto setRLE(bool enable) -> bool;
    // pattern trigger (trigger source LA): bits 7:0 the P lines, 15:8 the N lines
    auto setPattern(uint16_t mask, uint16_t value, uint16_t posEdge, uint16_t negEdge) -> bool;
    auto setTriggerSources(uint32_t mask) -> bool;
    auto setEventSource(uint32_t source) -> bool;
    auto setDataSize(uint32_t bytes) -> bool;
    auto setInAddress(uint8_t index, uint32_t addr) -> bool;
    auto setOutStep(uint32_t step) -> bool;

    // Hands an output buffer to the DMA (also while running, once the buffer is played out).
    auto setOutAddress(uint8_t index, uint32_t addr) -> bool;
    auto isOutBufferDone(uint8_t index) -> bool;

    // prepare() selects the event source (the block's own events by default), resets the DMA
    // engines and selects streaming mode, it clears the output buffers handed over before;
    // start() sends the start event and starts the engines.
    auto prepare(bool in, bool out) -> void;
    auto start(bool in, bool out) -> void;
    auto stop() -> void;
    auto softwareTrigger() -> void;
    auto isRunning() -> bool;

    // Trigger mode of the output: once its FIFO is full it plays from the next rising edge of
    // the trigger (mask 0x84 or the trigger event), as the DAC does. Refused while running.
    auto hasTriggerMode() -> bool;
    auto setOutputTriggerMode(bool enable) -> bool;
    auto isOutputArmed() -> bool;
    // Trigger mode and block mode of the input, as the scope: the start arms the input, nothing
    // is written until a rising edge of the trigger, which writes `blocks` buffers (a trigger
    // inside a series writes the current buffer and `blocks` more); then it waits again.
    // blocks = 0 writes everything after the trigger. The samples not written are counted in
    // a 64 bit lost counter per buffer. Refused while running.
    auto hasInputBlockMode() -> bool;
    auto setInputTriggerMode(bool enable) -> bool;
    auto isInputArmed() -> bool;
    auto setTriggerBlocks(uint16_t blocks) -> bool;
    auto getTriggerBlocks() -> uint16_t;
    auto getInputTriggerCount() -> uint32_t;
    // samples lost right before input buffer `index` (words of RLE count c stand for c + 1 samples)
    auto getInLost64(uint8_t index) -> uint64_t;

    auto waitIn(uint8_t index, int timeoutMs) -> bool;
    auto clearInBuffer(uint8_t index) -> void;
    auto getInLost(uint8_t index) -> uint32_t;
    auto getInWritePointer() -> uint32_t;
    auto getOutReadPointer() -> uint32_t;
    auto getInStatus() -> uint32_t;
    auto getOutStatus() -> uint32_t;
    auto getAcqStatus() -> uint32_t;
    auto getMap() -> volatile GpioMapT*;
    auto printReg() -> void;

   private:
    CGpio(const CGpio&) = delete;
    CGpio(CGpio&&) = delete;
    CGpio& operator=(const CGpio&) = delete;
    CGpio& operator=(const CGpio&&) = delete;

    int m_Fd;
    void* m_Regset;
    size_t m_RegsetSize;
    volatile GpioMapT* m_Map;
    std::mutex m_lock;
    bool m_running;
    uint32_t m_eventSource;
};

}  // namespace uio_lib

#endif
