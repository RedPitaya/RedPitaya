#include "ext_trigger.h"

using namespace uio_lib;

auto CExtTrigger::create(const std::vector<UioT>&) -> Ptr {
    return Ptr();
}

auto CExtTrigger::createDevMem() -> Ptr {
    return Ptr();
}

CExtTrigger::CExtTrigger(int _fd, void* _regset, size_t _regsetSize)
    : m_Fd(_fd),
      m_Regset(_regset),
      m_RegsetSize(_regsetSize),
      m_Reg(nullptr),
      m_lock(),
      m_caps(),
      m_condition(false),
      m_falling(false),
      m_debounce(false),
      m_debounceLen(0),
      m_sim(false),
      m_simLevel(false),
      m_dio0Disable(false) {}

CExtTrigger::~CExtTrigger() {}

auto CExtTrigger::probe() -> void {}

auto CExtTrigger::write() -> void {}

auto CExtTrigger::getCaps() const -> ExtTriggerCapsT {
    return m_caps;
}

auto CExtTrigger::getMaxDebounce() const -> uint32_t {
    return 0;
}

auto CExtTrigger::setConditioner(bool) -> void {}

auto CExtTrigger::setEdge(bool) -> void {}

auto CExtTrigger::setDebounce(bool, uint32_t) -> bool {
    return false;
}

auto CExtTrigger::setSimulation(bool) -> bool {
    return false;
}

auto CExtTrigger::setSimulationLevel(bool) -> bool {
    return false;
}

auto CExtTrigger::setDio0Trigger(bool) -> bool {
    return false;
}

auto CExtTrigger::reset() -> void {}

auto CExtTrigger::readRaw() -> uint32_t {
    return 0;
}

auto CExtTrigger::printReg() -> void {}
