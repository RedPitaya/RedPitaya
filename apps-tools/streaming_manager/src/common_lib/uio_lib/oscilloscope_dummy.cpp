#include <fcntl.h>
#include <cstdint>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "oscilloscope.h"
#include "oscilloscope_dummy.h"
#include "trigger.h"

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

auto COscilloscope::setDataSize(uint32_t size) -> void {
    dummy::oscState().dataSize = size;
}

void COscilloscope::setReg(volatile OscilloscopeMapT*) {}

void COscilloscope::setFilterCalibration(uint8_t ch, int32_t aa, int32_t bb, int32_t kk, int32_t pp) {
    auto& c = dummy::oscState().calibration[ch & 3];
    c.aa = aa, c.bb = bb, c.kk = kk, c.pp = pp;
}

void COscilloscope::setFilterBypass(bool state) {
    dummy::oscState().filterBypass = state;
}

auto COscilloscope::prepare() -> void {
    auto& s = dummy::oscState();
    s.prepares++;
    s.running = true;
}

auto COscilloscope::setCalibration(uint8_t ch, int32_t offset, float gain, bool isCalibV6) -> void {
    auto& c = dummy::oscState().calibration[ch & 3];
    c.offset = offset, c.gain = gain, c.beforeFilter = isCalibV6;
}

auto COscilloscope::getFPGAInfo(uint8_t, uint32_t& _overFlow, int64_t& _time) -> bool {
    _time = 0;
    _overFlow = 0;
    return true;
}

auto COscilloscope::getBufferTime(uint8_t index) -> int64_t {
    auto& s = dummy::oscState();
    return s.time ? s.time(index) : 0;
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

auto COscilloscope::setDecimation(uint32_t dec) -> bool {
    if (dec == 0 || dummy::oscState().running)
        return false;
    m_dec_factor = dec;
    dummy::oscState().decimation = dec;
    return true;
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

auto COscilloscope::isMaster() -> BoardMode {
    return BoardMode::UNKNOWN;
}

auto COscilloscope::stop() -> void {
    auto& s = dummy::oscState();
    s.stops++;
    s.running = false;
}

auto COscilloscope::printReg() -> void {}

auto COscilloscope::set8BitMode(bool mode) -> void {
    dummy::oscState().eightBit = mode;
}

// the setters refuse while running, as the real ones
auto COscilloscope::setTriggerMode(bool enable) -> bool {
    auto& s = dummy::oscState();
    if (s.running)
        return false;
    s.triggerMode = enable;
    return true;
}

auto COscilloscope::getTriggerMode() -> bool {
    return dummy::oscState().triggerMode;
}

auto COscilloscope::isArmed() -> bool {
    return dummy::oscState().armed;
}

auto COscilloscope::setTriggerSources(uint32_t mask) -> bool {
    auto& s = dummy::oscState();
    if (s.running || (mask & ~TRIG_SRC_ALL))
        return false;
    s.triggerSources = mask;
    return true;
}

auto COscilloscope::getTriggerSources() -> uint32_t {
    return dummy::oscState().triggerSources;
}

auto COscilloscope::setTriggerLevel(int32_t low, int32_t high) -> bool {
    auto& s = dummy::oscState();
    if (s.running || low > high || low < INT16_MIN || high > INT16_MAX)
        return false;
    s.triggerLow = low;
    s.triggerHigh = high;
    return true;
}

auto COscilloscope::setTriggerEdge(bool falling) -> bool {
    auto& s = dummy::oscState();
    if (s.running)
        return false;
    s.triggerFalling = falling;
    return true;
}

auto COscilloscope::setEventSource(uint32_t source) -> bool {
    auto& s = dummy::oscState();
    if (s.running || source > EVENT_SRC_MAX)
        return false;
    s.eventSource = source;
    return true;
}

auto COscilloscope::softwareTrigger() -> void {
    dummy::oscState().softwareTriggers++;
}

auto COscilloscope::isRunning() -> bool {
    return dummy::oscState().running;
}

auto COscilloscope::hasBlockMode() -> bool {
    return dummy::oscState().blockMode;
}

auto COscilloscope::setTriggerBlocks(uint16_t blocks) -> bool {
    auto& s = dummy::oscState();
    if (s.running)
        return false;
    s.triggerBlocks = blocks;
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
