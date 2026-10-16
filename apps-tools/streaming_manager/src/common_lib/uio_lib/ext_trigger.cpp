#include "ext_trigger.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

#include "logger_lib/file_logger.h"

using namespace uio_lib;

namespace {

constexpr uint32_t BIT_CONDITION = 1 << 0;
constexpr uint32_t BIT_FALLING = 1 << 1;
constexpr uint32_t BIT_DEBOUNCE = 1 << 2;
constexpr uint32_t BIT_SIM = 1 << 4;
constexpr uint32_t BIT_SIM_LEVEL = 1 << 5;
constexpr uint32_t BIT_DIO0_DISABLE = 1 << 6;
constexpr uint32_t LEN8_SHIFT = 8;
constexpr uint32_t LEN16_SHIFT = 16;

}  // namespace

auto CExtTrigger::create(const std::vector<UioT>& uioList) -> Ptr {
    for (const auto& uio : uioList) {
        if (uio.nodeName != "rp_gpio@40200000" && uio.nodeName != "rp_ext_trig@40200000")
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
        return std::make_shared<CExtTrigger>(fd, regset, uio.mapList[0].size);
    }
    return Ptr();
}

auto CExtTrigger::createDevMem() -> Ptr {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        ERROR_LOG("Error: open /dev/mem")
        return Ptr();
    }
    size_t size = getpagesize();
    void* regset = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base_address);
    if (regset == MAP_FAILED) {
        ERROR_LOG("Error: mmap 0x%X", (uint32_t)base_address)
        close(fd);
        return Ptr();
    }
    return std::make_shared<CExtTrigger>(fd, regset, size);
}

CExtTrigger::CExtTrigger(int _fd, void* _regset, size_t _regsetSize)
    : m_Fd(_fd),
      m_Regset(_regset),
      m_RegsetSize(_regsetSize),
      m_Reg(reinterpret_cast<volatile uint32_t*>(reinterpret_cast<uintptr_t>(_regset) + reg_offset)),
      m_lock(),
      m_caps(),
      m_condition(false),
      m_falling(false),
      m_debounce(false),
      m_debounceLen(0),
      m_sim(false),
      m_simLevel(false),
      m_dio0Disable(false) {
    probe();
}

CExtTrigger::~CExtTrigger() {
    munmap(m_Regset, m_RegsetSize);
    close(m_Fd);
}

auto CExtTrigger::probe() -> void {
    std::lock_guard lock(m_lock);
    uint32_t orig = *m_Reg;
    // With the conditioner off and the simulated level low the probe cannot produce a trigger edge.
    *m_Reg = (1 << LEN16_SHIFT) | BIT_SIM | BIT_DIO0_DISABLE;
    uint32_t rd = *m_Reg;
    *m_Reg = orig;
    m_caps.debounce16 = rd & (1 << LEN16_SHIFT);
    m_caps.simulation = rd & BIT_SIM;
    m_caps.dio0Disable = rd & BIT_DIO0_DISABLE;

    m_condition = orig & BIT_CONDITION;
    m_falling = orig & BIT_FALLING;
    m_debounce = orig & BIT_DEBOUNCE;
    m_debounceLen = m_caps.debounce16 ? (orig >> LEN16_SHIFT) & 0xFFFF : (orig >> LEN8_SHIFT) & 0xFF;
    m_sim = m_caps.simulation && (orig & BIT_SIM);
    m_simLevel = m_caps.simulation && (orig & BIT_SIM_LEVEL);
    m_dio0Disable = m_caps.dio0Disable && (orig & BIT_DIO0_DISABLE);
}

auto CExtTrigger::write() -> void {
    uint32_t value = (m_condition ? BIT_CONDITION : 0) | (m_falling ? BIT_FALLING : 0) | (m_debounce ? BIT_DEBOUNCE : 0);
    value |= m_caps.debounce16 ? (m_debounceLen << LEN16_SHIFT) : (m_debounceLen << LEN8_SHIFT);
    if (m_caps.simulation)
        value |= (m_sim ? BIT_SIM : 0) | (m_simLevel ? BIT_SIM_LEVEL : 0);
    if (m_caps.dio0Disable)
        value |= m_dio0Disable ? BIT_DIO0_DISABLE : 0;
    *m_Reg = value;
}

auto CExtTrigger::getCaps() const -> ExtTriggerCapsT {
    return m_caps;
}

auto CExtTrigger::getMaxDebounce() const -> uint32_t {
    return m_caps.debounce16 ? 0xFFFF : 0xFF;
}

auto CExtTrigger::setConditioner(bool enable) -> void {
    std::lock_guard lock(m_lock);
    m_condition = enable;
    write();
}

auto CExtTrigger::setEdge(bool falling) -> void {
    std::lock_guard lock(m_lock);
    m_falling = falling;
    write();
}

auto CExtTrigger::setDebounce(bool enable, uint32_t lengthClk) -> bool {
    if (lengthClk > getMaxDebounce())
        return false;
    std::lock_guard lock(m_lock);
    m_debounce = enable;
    m_debounceLen = lengthClk;
    write();
    return true;
}

auto CExtTrigger::setSimulation(bool enable) -> bool {
    if (!m_caps.simulation)
        return false;
    std::lock_guard lock(m_lock);
    m_sim = enable;
    write();
    return true;
}

auto CExtTrigger::setSimulationLevel(bool level) -> bool {
    if (!m_caps.simulation)
        return false;
    std::lock_guard lock(m_lock);
    m_simLevel = level;
    write();
    return true;
}

auto CExtTrigger::setDio0Trigger(bool enable) -> bool {
    if (!m_caps.dio0Disable)
        return enable;
    std::lock_guard lock(m_lock);
    m_dio0Disable = !enable;
    write();
    return true;
}

auto CExtTrigger::reset() -> void {
    std::lock_guard lock(m_lock);
    m_condition = m_falling = m_debounce = m_sim = m_simLevel = m_dio0Disable = false;
    m_debounceLen = 0;
    write();
}

auto CExtTrigger::readRaw() -> uint32_t {
    return *m_Reg;
}

auto CExtTrigger::printReg() -> void {
    fprintf(stderr, "0x%X ext_trig = 0x%X (debounce16 %d, simulation %d, dio0Disable %d)\n", (uint32_t)(base_address + reg_offset), *m_Reg, m_caps.debounce16,
            m_caps.simulation, m_caps.dio0Disable);
}
