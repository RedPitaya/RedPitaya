#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "oscilloscope.h"
#include "oscilloscope_dummy.h"

using namespace uio_lib;

auto uio_lib::dummy::oscState() -> OscState& {
    static OscState state;
    return state;
}

auto uio_lib::dummy::resetOscState() -> void {
    oscState() = OscState();
}

auto COscilloscope::create(const UioT&, uint32_t _dec_factor, bool _isMaster, uint32_t _adcMaxSpeed, bool _isADCFilterPresent, uint8_t _fpgaBits,
                           uint8_t _maxChannels) -> COscilloscope::Ptr {
    return std::make_shared<COscilloscope>(0, nullptr, 0, _dec_factor, _isMaster, _adcMaxSpeed, _isADCFilterPresent, _fpgaBits, _maxChannels);
}

COscilloscope::COscilloscope(int _fd, void* _regset, size_t _regsetSize, uint32_t _dec_factor, bool _isMaster, uint32_t _adcMaxSpeed, bool _isADCFilterPresent, uint8_t _fpgaBits,
                             uint8_t _maxChannels)
    : m_Fd(_fd),
      m_Regset(_regset),
      m_RegsetSize(_regsetSize),
      m_OscMap(nullptr),
      m_dec_factor(_dec_factor),
      m_filterBypass(true),
      m_isMaster(_isMaster),
      m_adcMaxSpeed(_adcMaxSpeed),
      m_isADCFilterPresent(_isADCFilterPresent),
      m_fpgaBits(_fpgaBits),
      m_maxChannels(_maxChannels) {}

COscilloscope::~COscilloscope() {}

auto COscilloscope::setSkipDataAddress(uio_lib::MemoryRegionT) -> void {}

auto COscilloscope::setDataAddress(uint8_t, uint32_t, uint32_t, uint32_t, uint32_t) -> void {}

auto COscilloscope::setDataSize(uint32_t) -> void {}

void COscilloscope::setReg(volatile OscilloscopeMapT*) {}

void COscilloscope::setFilterCalibration(uint8_t, int32_t, int32_t, int32_t, int32_t) {}

void COscilloscope::setFilterBypass(bool) {}

auto COscilloscope::prepare() -> void {}

auto COscilloscope::setCalibration(uint8_t, int32_t, float, bool) -> void {}

auto COscilloscope::getFPGAInfo(uint8_t, uint32_t& _overFlow, int64_t& _time) -> bool {
    _time = 0;
    _overFlow = 0;
    return true;
}

auto COscilloscope::getBufferTime(uint8_t) -> int64_t {
    return 0;
}

auto COscilloscope::getBufferWait(uint8_t index) -> uint64_t {
    auto& s = dummy::oscState();
    return s.waitCount ? s.waitCount(index) : 0;
}

auto COscilloscope::getFeatures() -> uint32_t {
    return 0;
}

auto COscilloscope::hasWaitCounter() -> bool {
    return dummy::oscState().waitCounter;
}

auto COscilloscope::getDecimation() -> uint32_t {
    return m_dec_factor;
}

auto COscilloscope::getOSCRate() -> uint32_t {
    return m_dec_factor;
}

auto COscilloscope::getADCClock() -> uint32_t {
    return m_adcMaxSpeed;
}

auto COscilloscope::clearBuffer(uint8_t) -> bool {
    return true;
}

auto COscilloscope::wait() -> bool {
    auto& s = dummy::oscState();
    return s.wait ? s.wait() : true;
}

auto COscilloscope::clearInterrupt() -> bool {
    return true;
}

auto isMaster() -> BoardMode {
    return BoardMode::UNKNOWN;
}

auto COscilloscope::stop() -> void {}

auto COscilloscope::printReg() -> void {}

auto COscilloscope::set8BitMode(bool) -> void {}

auto COscilloscope::setTriggerMode(bool) -> bool {
    return true;
}

auto COscilloscope::getTriggerMode() -> bool {
    return dummy::oscState().triggerMode;
}

auto COscilloscope::isArmed() -> bool {
    return false;
}

auto COscilloscope::setTriggerSources(uint32_t) -> bool {
    return true;
}

auto COscilloscope::getTriggerSources() -> uint32_t {
    return 0;
}

auto COscilloscope::setTriggerLevel(int32_t, int32_t) -> bool {
    return true;
}

auto COscilloscope::setTriggerEdge(bool) -> bool {
    return true;
}

auto COscilloscope::setEventSource(uint32_t) -> bool {
    return true;
}

auto COscilloscope::softwareTrigger() -> void {}

auto COscilloscope::isRunning() -> bool {
    return false;
}

auto COscilloscope::hasBlockMode() -> bool {
    return false;
}

auto COscilloscope::setTriggerBlocks(uint16_t) -> bool {
    return true;
}

auto COscilloscope::getTriggerBlocks() -> uint16_t {
    return dummy::oscState().triggerBlocks;
}

auto COscilloscope::getTriggerCount() -> uint32_t {
    auto& s = dummy::oscState();
    return s.triggerCount ? s.triggerCount() : 0;
}

auto COscilloscope::getBufferLost(uint8_t index, uint8_t ch) -> uint64_t {
    auto& s = dummy::oscState();
    return s.lost ? s.lost(index, ch) : 0;
}
