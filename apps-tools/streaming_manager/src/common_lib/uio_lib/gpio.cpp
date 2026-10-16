#include "gpio.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <chrono>
#include <string>

#include "logger_lib/file_logger.h"

using namespace uio_lib;

namespace {

// input DMA control (0x8C), the same engine as the scope
constexpr uint32_t IN_CTRL_START = 1 << 0;
constexpr uint32_t IN_CTRL_INTR_ACK = 1 << 1;
constexpr uint32_t IN_CTRL_BUF1_ACK = 1 << 2;
constexpr uint32_t IN_CTRL_BUF2_ACK = 1 << 3;
constexpr uint32_t IN_CTRL_RESET = 1 << 4;
constexpr uint32_t IN_CTRL_MODE_STREAM = 1 << 9;
constexpr uint32_t IN_STS_BUF1_FULL = 1 << 0;
constexpr uint32_t IN_STS_BUF2_FULL = 1 << 1;

// output DMA control (0x90), the same engine as one DAC channel
constexpr uint32_t OUT_CTRL_START = 1 << 0;
constexpr uint32_t OUT_CTRL_RESET = 1 << 1;
constexpr uint32_t OUT_CTRL_MODE_STREAM = 1 << 5;
constexpr uint32_t OUT_CTRL_BUF1_RDY = 1 << 6;
constexpr uint32_t OUT_CTRL_BUF2_RDY = 1 << 7;
constexpr uint32_t OUT_STS_BUF1_DONE = 1 << 7;
constexpr uint32_t OUT_STS_BUF2_DONE = 1 << 8;

constexpr uint32_t MAX_DECIMATION = 1 << 17;

}  // namespace

auto CGpio::create(const std::vector<UioT>& uioList) -> Ptr {
    for (const auto& uio : uioList) {
        if (uio.nodeName != "rp_gpio@40200000")
            continue;
        if (uio.mapList.empty()) {
            ERROR_LOG("Error: UIO validation.")
            return Ptr();
        }
        std::string path("/dev/" + uio.name);
        int fd = open(path.c_str(), O_RDWR);
        if (fd == -1) {
            ERROR_LOG("Error: open file %s", path.c_str())
            return Ptr();
        }
        void* regset = mmap(nullptr, uio.mapList[0].size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (regset == MAP_FAILED) {
            ERROR_LOG("Error: mmap regset.")
            close(fd);
            return Ptr();
        }
        return std::make_shared<CGpio>(fd, regset, uio.mapList[0].size);
    }
    return Ptr();
}

CGpio::CGpio(int _fd, void* _regset, size_t _regsetSize)
    : m_Fd(_fd),
      m_Regset(_regset),
      m_RegsetSize(_regsetSize),
      m_Map(reinterpret_cast<volatile GpioMapT*>(_regset)),
      m_lock(),
      m_running(false),
      m_eventSource(gpio_event_id) {}

CGpio::~CGpio() {
    stop();
    munmap(m_Regset, m_RegsetSize);
    close(m_Fd);
}

auto CGpio::setOutputs(uint8_t p, uint8_t n) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running)
        return false;
    // the direction registers drive the IOBUF tristate input: 1 is high impedance
    m_Map->dir_p = (uint8_t)~p;
    m_Map->dir_n = (uint8_t)~n;
    return true;
}

auto CGpio::setPolarity(uint16_t mask) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running)
        return false;
    m_Map->polarity = mask;
    return true;
}

auto CGpio::setDecimation(uint32_t dec) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running || dec == 0 || dec > MAX_DECIMATION)
        return false;
    m_Map->decimation = dec - 1;  // the counter reloads with the value: period = value + 1
    return true;
}

auto CGpio::setRLE(bool enable) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running)
        return false;
    m_Map->rle = enable ? 1 : 0;
    return true;
}

auto CGpio::setPattern(uint16_t mask, uint16_t value, uint16_t posEdge, uint16_t negEdge) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running)
        return false;
    m_Map->cmp_mask = mask;
    m_Map->cmp_value = value;
    m_Map->edge_pos = posEdge;
    m_Map->edge_neg = negEdge;
    return true;
}

auto CGpio::setTriggerSources(uint32_t mask) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running || (mask & ~TRIG_SRC_ALL))
        return false;
    m_Map->trig_mask = mask;
    return true;
}

auto CGpio::setEventSource(uint32_t source) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running || source > EVENT_SRC_MAX)
        return false;
    m_eventSource = source;
    return true;
}

auto CGpio::setDataSize(uint32_t bytes) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running || bytes == 0 || (bytes % 128))
        return false;
    m_Map->dma_size = bytes;
    return true;
}

auto CGpio::setInAddress(uint8_t index, uint32_t addr) -> bool {
    std::lock_guard lock(m_lock);
    if (index > 1)
        return false;
    if (index == 0)
        m_Map->buf1_in = addr;
    else
        m_Map->buf2_in = addr;
    return true;
}

auto CGpio::setOutStep(uint32_t step) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running || step == 0)
        return false;
    m_Map->out_step = step;
    return true;
}

auto CGpio::setOutAddress(uint8_t index, uint32_t addr) -> bool {
    std::lock_guard lock(m_lock);
    if (index > 1)
        return false;
    if (index == 0) {
        m_Map->buf1_out = addr;
        m_Map->dma_ctrl_out = OUT_CTRL_BUF1_RDY;
    } else {
        m_Map->buf2_out = addr;
        m_Map->dma_ctrl_out = OUT_CTRL_BUF2_RDY;
    }
    return true;
}

auto CGpio::isOutBufferDone(uint8_t index) -> bool {
    return m_Map->dma_sts_out & (index == 0 ? OUT_STS_BUF1_DONE : OUT_STS_BUF2_DONE);
}

auto CGpio::prepare(bool in, bool out) -> void {
    stop();
    std::lock_guard lock(m_lock);
    m_Map->event_sel = m_eventSource;
    if (in)
        m_Map->dma_ctrl_in = IN_CTRL_MODE_STREAM | IN_CTRL_RESET | IN_CTRL_BUF2_ACK | IN_CTRL_BUF1_ACK | IN_CTRL_INTR_ACK;
    if (out)
        m_Map->dma_ctrl_out = OUT_CTRL_MODE_STREAM | OUT_CTRL_RESET;
}

auto CGpio::start(bool in, bool out) -> void {
    std::lock_guard lock(m_lock);
    m_Map->event = GPIO_EVENT_START;
    if (in)
        m_Map->dma_ctrl_in = IN_CTRL_START;
    if (out)
        m_Map->dma_ctrl_out = OUT_CTRL_START;
    m_running = in || out;
}

auto CGpio::stop() -> void {
    std::lock_guard lock(m_lock);
    m_Map->event = GPIO_EVENT_STOP;
    m_Map->dma_ctrl_out = 0;
    m_running = false;
}

auto CGpio::softwareTrigger() -> void {
    std::lock_guard lock(m_lock);
    m_Map->event = GPIO_EVENT_TRIG;
}

auto CGpio::isRunning() -> bool {
    return m_running;
}

auto CGpio::hasTriggerMode() -> bool {
    std::lock_guard lock(m_lock);
    // older bitstreams have no register at 0x08: it reads back 0
    uint32_t old = m_Map->trig_mode;
    m_Map->trig_mode = 0x2;
    bool present = (m_Map->trig_mode & 0x3) == 0x2;
    m_Map->trig_mode = old & 0x3;
    return present;
}

auto CGpio::setOutputTriggerMode(bool enable) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running)
        return false;
    uint32_t mode = m_Map->trig_mode & 0x3;
    m_Map->trig_mode = enable ? (mode | 0x2) : (mode & ~0x2u);
    return true;
}

auto CGpio::isOutputArmed() -> bool {
    return (m_Map->trig_mode >> 9) & 0x1;
}

auto CGpio::hasInputBlockMode() -> bool {
    std::lock_guard lock(m_lock);
    // older bitstreams have no register at 0x0C: it reads back 0
    uint32_t old = m_Map->trig_blocks;
    m_Map->trig_blocks = 0x5A5A;
    bool present = (m_Map->trig_blocks & 0xFFFF) == 0x5A5A;
    m_Map->trig_blocks = old & 0xFFFF;
    return present;
}

auto CGpio::setInputTriggerMode(bool enable) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running)
        return false;
    uint32_t mode = m_Map->trig_mode & 0x3;
    m_Map->trig_mode = enable ? (mode | 0x1) : (mode & ~0x1u);
    return true;
}

auto CGpio::isInputArmed() -> bool {
    return (m_Map->trig_mode >> 8) & 0x1;
}

auto CGpio::setTriggerBlocks(uint16_t blocks) -> bool {
    std::lock_guard lock(m_lock);
    if (m_running)
        return false;
    m_Map->trig_blocks = blocks;
    return true;
}

auto CGpio::getTriggerBlocks() -> uint16_t {
    return m_Map->trig_blocks & 0xFFFF;
}

auto CGpio::getInputTriggerCount() -> uint32_t {
    return m_Map->in_trig_count;
}

auto CGpio::getInLost64(uint8_t index) -> uint64_t {
    if (index > 1)
        return 0;
    // the pair is latched at the first write request of the buffer, long before it is full
    uint32_t lo = m_Map->in_lost[index][0];
    uint32_t hi = m_Map->in_lost[index][1];
    return (uint64_t)hi << 32 | lo;
}

auto CGpio::waitIn(uint8_t index, int timeoutMs) -> bool {
    const uint32_t mask = index == 0 ? IN_STS_BUF1_FULL : IN_STS_BUF2_FULL;
    auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (!(m_Map->dma_sts_in & mask)) {
        if (std::chrono::steady_clock::now() > end)
            return false;
        usleep(20);
    }
    return true;
}

auto CGpio::clearInBuffer(uint8_t index) -> void {
    std::lock_guard lock(m_lock);
    m_Map->dma_ctrl_in = index == 0 ? IN_CTRL_BUF1_ACK : IN_CTRL_BUF2_ACK;
}

auto CGpio::getInLost(uint8_t index) -> uint32_t {
    return index == 0 ? m_Map->lost_buf1 : m_Map->lost_buf2;
}

auto CGpio::getInWritePointer() -> uint32_t {
    return m_Map->wp_in;
}

auto CGpio::getOutReadPointer() -> uint32_t {
    return m_Map->rp_out;
}

auto CGpio::getInStatus() -> uint32_t {
    return m_Map->dma_sts_in;
}

auto CGpio::getOutStatus() -> uint32_t {
    return m_Map->dma_sts_out;
}

auto CGpio::getAcqStatus() -> uint32_t {
    return m_Map->acq_sts;
}

auto CGpio::getMap() -> volatile GpioMapT* {
    return m_Map;
}

auto CGpio::printReg() -> void {
    fprintf(stderr, "printReg\n");
    fprintf(stderr, "0x00 acq_sts = 0x%X\n", m_Map->acq_sts);
    fprintf(stderr, "0x40 cmp_mask = 0x%X\n", m_Map->cmp_mask);
    fprintf(stderr, "0x44 cmp_value = 0x%X\n", m_Map->cmp_value);
    fprintf(stderr, "0x48 edge_pos = 0x%X\n", m_Map->edge_pos);
    fprintf(stderr, "0x4C edge_neg = 0x%X\n", m_Map->edge_neg);
    fprintf(stderr, "0x50 decimation = 0x%X\n", m_Map->decimation);
    fprintf(stderr, "0x54 rle = 0x%X\n", m_Map->rle);
    fprintf(stderr, "0x58 cnt_current = 0x%X\n", m_Map->cnt_current);
    fprintf(stderr, "0x5C cnt_last = 0x%X\n", m_Map->cnt_last);
    fprintf(stderr, "0x60 polarity = 0x%X\n", m_Map->polarity);
    fprintf(stderr, "0x68 ext_trig = 0x%X\n", m_Map->ext_trig);
    fprintf(stderr, "0x70 dir_p = 0x%X\n", m_Map->dir_p);
    fprintf(stderr, "0x74 dir_n = 0x%X\n", m_Map->dir_n);
    fprintf(stderr, "0x80 event_sel = 0x%X\n", m_Map->event_sel);
    fprintf(stderr, "0x84 trig_mask = 0x%X\n", m_Map->trig_mask);
    fprintf(stderr, "0x8C dma_ctrl_in = 0x%X\n", m_Map->dma_ctrl_in);
    fprintf(stderr, "0x90 dma_ctrl_out = 0x%X\n", m_Map->dma_ctrl_out);
    fprintf(stderr, "0x94 dma_sts_in = 0x%X\n", m_Map->dma_sts_in);
    fprintf(stderr, "0x98 dma_sts_out = 0x%X\n", m_Map->dma_sts_out);
    fprintf(stderr, "0x9C dma_size = 0x%X\n", m_Map->dma_size);
    fprintf(stderr, "0xA0 buf1_in = 0x%X\n", m_Map->buf1_in);
    fprintf(stderr, "0xA4 buf1_out = 0x%X\n", m_Map->buf1_out);
    fprintf(stderr, "0xA8 buf2_in = 0x%X\n", m_Map->buf2_in);
    fprintf(stderr, "0xAC buf2_out = 0x%X\n", m_Map->buf2_out);
    fprintf(stderr, "0xB0 lost_buf1 = 0x%X\n", m_Map->lost_buf1);
    fprintf(stderr, "0xB4 lost_buf2 = 0x%X\n", m_Map->lost_buf2);
    fprintf(stderr, "0xB8 wp_in = 0x%X\n", m_Map->wp_in);
    fprintf(stderr, "0xBC rp_out = 0x%X\n", m_Map->rp_out);
    fprintf(stderr, "0xC0 out_step = 0x%X\n", m_Map->out_step);
    fprintf(stderr, "0xC4 in_dma_diag = 0x%X\n", m_Map->in_dma_diag);
}
