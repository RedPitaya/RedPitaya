#include "gpio.h"

using namespace uio_lib;

auto CGpio::create(const std::vector<UioT>&) -> Ptr {
    return Ptr();
}

CGpio::CGpio(int _fd, void* _regset, size_t _regsetSize) : m_Fd(_fd), m_Regset(_regset), m_RegsetSize(_regsetSize), m_Map(nullptr), m_lock(), m_running(false), m_eventSource(gpio_event_id) {}

CGpio::~CGpio() {}

auto CGpio::setOutputs(uint8_t, uint8_t) -> bool {
    return false;
}

auto CGpio::setPolarity(uint16_t) -> bool {
    return false;
}

auto CGpio::setDecimation(uint32_t) -> bool {
    return false;
}

auto CGpio::setRLE(bool) -> bool {
    return false;
}

auto CGpio::setPattern(uint16_t, uint16_t, uint16_t, uint16_t) -> bool {
    return false;
}

auto CGpio::setTriggerSources(uint32_t) -> bool {
    return false;
}

auto CGpio::setEventSource(uint32_t) -> bool {
    return false;
}

auto CGpio::setDataSize(uint32_t) -> bool {
    return false;
}

auto CGpio::setInAddress(uint8_t, uint32_t) -> bool {
    return false;
}

auto CGpio::setOutStep(uint32_t) -> bool {
    return false;
}

auto CGpio::setOutAddress(uint8_t, uint32_t) -> bool {
    return false;
}

auto CGpio::isOutBufferDone(uint8_t) -> bool {
    return false;
}

auto CGpio::prepare(bool, bool) -> void {}

auto CGpio::start(bool, bool) -> void {}

auto CGpio::stop() -> void {}

auto CGpio::softwareTrigger() -> void {}

auto CGpio::isRunning() -> bool {
    return false;
}

auto CGpio::hasTriggerMode() -> bool {
    return false;
}

auto CGpio::setOutputTriggerMode(bool) -> bool {
    return false;
}

auto CGpio::isOutputArmed() -> bool {
    return false;
}

auto CGpio::hasInputBlockMode() -> bool {
    return false;
}

auto CGpio::setInputTriggerMode(bool) -> bool {
    return false;
}

auto CGpio::isInputArmed() -> bool {
    return false;
}

auto CGpio::setTriggerBlocks(uint16_t) -> bool {
    return false;
}

auto CGpio::getTriggerBlocks() -> uint16_t {
    return 0;
}

auto CGpio::getInputTriggerCount() -> uint32_t {
    return 0;
}

auto CGpio::getInLost64(uint8_t) -> uint64_t {
    return 0;
}

auto CGpio::waitIn(uint8_t, int) -> bool {
    return false;
}

auto CGpio::clearInBuffer(uint8_t) -> void {}

auto CGpio::getInLost(uint8_t) -> uint32_t {
    return 0;
}

auto CGpio::getInWritePointer() -> uint32_t {
    return 0;
}

auto CGpio::getOutReadPointer() -> uint32_t {
    return 0;
}

auto CGpio::getInStatus() -> uint32_t {
    return 0;
}

auto CGpio::getOutStatus() -> uint32_t {
    return 0;
}

auto CGpio::getAcqStatus() -> uint32_t {
    return 0;
}

auto CGpio::getMap() -> volatile GpioMapT* {
    return nullptr;
}

auto CGpio::printReg() -> void {}
