#ifndef UIO_LIB_EXT_TRIGGER_H
#define UIO_LIB_EXT_TRIGGER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>
#include "uio_parser.h"

namespace uio_lib {

// Features of the external trigger conditioner found in the loaded bitstream.
struct ExtTriggerCapsT {
    bool debounce16{false};   // debounce length is 16 bit wide in bits 31:16 (8 bit in bits 15:8 otherwise)
    bool simulation{false};   // bits 4/5 replace the pin with a register level
    bool dio0Disable{false};  // bit 6 disconnects DIO0_P from the external trigger
};

// External trigger conditioner, register 0x68 of the GPIO block (0x40200068).
// On the 250-12 the same register sits in a separate slave without the GPIO block.
class CExtTrigger {
   public:
    using Ptr = std::shared_ptr<CExtTrigger>;

    static constexpr uintptr_t base_address = 0x40200000;
    static constexpr uint32_t reg_offset = 0x68;

    // Looks for the GPIO block (rp_gpio@40200000) or the 250-12 node (rp_ext_trig@40200000).
    static auto create(const std::vector<UioT>& uioList) -> Ptr;
    // For a 250-12 bitstream whose device tree has no node for the register: maps it through /dev/mem.
    // Only for a board where 0x40200000 is implemented, an unmapped AXI address hangs the bus.
    static auto createDevMem() -> Ptr;

    CExtTrigger(int _fd, void* _regset, size_t _regsetSize);
    ~CExtTrigger();

    auto getCaps() const -> ExtTriggerCapsT;
    auto getMaxDebounce() const -> uint32_t;

    // Disabled conditioner (reset default): the raw level of the pin goes to the trigger masks.
    // Enabled: a synchronised, optionally debounced one-clock pulse on the selected edge.
    auto setConditioner(bool enable) -> void;
    auto setEdge(bool falling) -> void;
    auto setDebounce(bool enable, uint32_t lengthClk) -> bool;
    auto setSimulation(bool enable) -> bool;
    auto setSimulationLevel(bool level) -> bool;
    auto setDio0Trigger(bool enable) -> bool;
    auto reset() -> void;
    auto readRaw() -> uint32_t;
    auto printReg() -> void;

   private:
    CExtTrigger(const CExtTrigger&) = delete;
    CExtTrigger(CExtTrigger&&) = delete;
    CExtTrigger& operator=(const CExtTrigger&) = delete;
    CExtTrigger& operator=(const CExtTrigger&&) = delete;

    auto probe() -> void;
    auto write() -> void;

    int m_Fd;
    void* m_Regset;
    size_t m_RegsetSize;
    volatile uint32_t* m_Reg;
    std::mutex m_lock;
    ExtTriggerCapsT m_caps;
    bool m_condition;
    bool m_falling;
    bool m_debounce;
    uint32_t m_debounceLen;
    bool m_sim;
    bool m_simLevel;
    bool m_dio0Disable;
};

}  // namespace uio_lib

#endif
