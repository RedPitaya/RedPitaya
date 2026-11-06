#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "generator.h"
#include "generator_dummy.h"

using namespace uio_lib;

auto uio_lib::dummy::genState() -> GenState& {
    static GenState state;
    return state;
}

auto uio_lib::dummy::resetGenState() -> void {
    genState() = GenState();
}

auto CGenerator::create(const UioT&, bool _channel1Enable, bool _channel2Enable, uint32_t dacHz, uint32_t maxDacHz) -> CGenerator::Ptr {
    return std::make_shared<CGenerator>(_channel1Enable, _channel2Enable, 0, nullptr, 0, dacHz, maxDacHz);
}

CGenerator::CGenerator(bool _channel1Enable, bool _channel2Enable, int _fd, void* _regset, size_t _regsetSize, uint32_t dacHz, uint32_t maxDacHz)
    : m_Channel1(_channel1Enable),
      m_Channel2(_channel2Enable),
      m_Fd(_fd),
      m_Regset(_regset),
      m_RegsetSize(_regsetSize),
      m_Map(nullptr),
      m_Buffer1(nullptr),
      m_Buffer2(nullptr),
      m_waitLock(),
      m_maxDacSpeedHz(maxDacHz),
      m_dacSpeedHz(dacHz),
      m_is8Bit{false, false} {}

CGenerator::~CGenerator() {}

auto CGenerator::getDacHz() -> uint32_t {
    return m_dacSpeedHz;
}

auto CGenerator::setDacHz(uint32_t hz) -> bool {
    if (((double)hz / (double)m_maxDacSpeedHz) * (1 << 16) < 1)
        return false;
    m_dacSpeedHz = hz;
    return true;
}

auto CGenerator::setReg(volatile GeneratorMapT*) -> void {}

auto CGenerator::prepare() -> void {}

auto CGenerator::setCalibration(int32_t, float, int32_t, float) -> void {}

auto CGenerator::setDataAddress(uint8_t index, uint32_t ch1, uint32_t ch2, uint32_t size) -> bool {
    auto& s = dummy::genState();
    if (s.slotFree && !s.slotFree(index))
        return false;
    if (s.taken)
        s.taken(index, ch1, ch2, size);
    return true;
}

auto CGenerator::setDataBits(bool, bool) -> void {}

auto CGenerator::setDataSize(uint32_t) -> void {}

auto CGenerator::start(bool, bool) -> void {
    dummy::genState().running = true;
    dummy::genState().starts++;
}

auto CGenerator::stop() -> void {
    dummy::genState().running = false;
    dummy::genState().stops++;
}

auto CGenerator::printReg() -> void {}

auto CGenerator::setTriggerMode(bool ch1, bool ch2) -> bool {
    dummy::genState().triggerMode[0] = ch1;
    dummy::genState().triggerMode[1] = ch2;
    dummy::genState().triggerSets++;
    return true;
}

auto CGenerator::getArmed() -> uint8_t {
    return 0;
}

auto CGenerator::setTriggerSources(uint32_t sources) -> bool {
    dummy::genState().triggerSources = sources;
    return true;
}

auto CGenerator::getTriggerSources() -> uint32_t {
    return dummy::genState().triggerSources;
}

auto CGenerator::setEventSource(uint32_t source) -> bool {
    dummy::genState().eventSource = source;
    return true;
}

auto CGenerator::softwareTrigger() -> void {}

auto CGenerator::isRunning() -> bool {
    return false;
}

auto CGenerator::hasLoopMode() -> bool {
    return false;
}

auto CGenerator::startLoop(uint32_t, uint32_t, uint32_t, uint32_t) -> void {}

auto CGenerator::isLoopDone() -> bool {
    return true;
}

auto CGenerator::getLoopPlayed() -> uint32_t {
    return 0;
}

auto CGenerator::waitPlayed(uint32_t) -> bool {
    return true;
}
