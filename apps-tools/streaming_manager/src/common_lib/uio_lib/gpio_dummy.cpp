#include <chrono>
#include <thread>

#include "gpio.h"
#include "gpio_dummy.h"

using namespace uio_lib;

auto uio_lib::dummy::gpioState() -> GpioState& {
    static GpioState state;
    return state;
}

auto uio_lib::dummy::resetGpioState() -> void {
    gpioState() = GpioState();
}

namespace {

auto S() -> dummy::GpioState& {
    return dummy::gpioState();
}

}  // namespace

auto CGpio::create(const std::vector<UioT>&) -> Ptr {
    return S().present ? std::make_shared<CGpio>(0, nullptr, 0) : Ptr();
}

CGpio::CGpio(int _fd, void* _regset, size_t _regsetSize) : m_Fd(_fd), m_Regset(_regset), m_RegsetSize(_regsetSize), m_Map(nullptr), m_lock(), m_running(false), m_eventSource(gpio_event_id) {}

CGpio::~CGpio() {}

auto CGpio::setOutputs(uint8_t p, uint8_t n) -> bool {
    S().outputsP = p;
    S().outputsN = n;
    return true;
}

auto CGpio::setPolarity(uint16_t mask) -> bool {
    S().polarity = mask;
    return true;
}

auto CGpio::setDecimation(uint32_t dec) -> bool {
    S().decimation = dec;
    return dec != 0;
}

auto CGpio::setRLE(bool enable) -> bool {
    S().rle = enable;
    return true;
}

auto CGpio::setPattern(uint16_t, uint16_t, uint16_t, uint16_t) -> bool {
    return true;
}

auto CGpio::setTriggerSources(uint32_t mask) -> bool {
    S().triggerSources = mask;
    return true;
}

auto CGpio::setEventSource(uint32_t) -> bool {
    return true;
}

auto CGpio::setDataSize(uint32_t bytes) -> bool {
    S().dataSize = bytes;
    return bytes != 0 && bytes % 128 == 0;
}

auto CGpio::setInAddress(uint8_t index, uint32_t addr) -> bool {
    S().inAddress[index & 1] = addr;
    return true;
}

auto CGpio::setOutStep(uint32_t step) -> bool {
    S().outStep = step;
    return step != 0;
}

auto CGpio::setOutAddress(uint8_t index, uint32_t addr) -> bool {
    if (S().outTaken)
        S().outTaken(index, addr);
    return true;
}

auto CGpio::isOutBufferDone(uint8_t index) -> bool {
    return !S().outDone || S().outDone(index);
}

auto CGpio::prepare(bool, bool) -> void {
    stop();
    S().prepares++;
    S().inFull = {false, false};
}

auto CGpio::start(bool in, bool out) -> void {
    S().in = in;
    S().out = out;
    S().running = in || out;
    S().starts++;
}

auto CGpio::stop() -> void {
    S().running = false;
    S().stops++;
}

auto CGpio::softwareTrigger() -> void {
    S().softwareTriggers++;
}

auto CGpio::isRunning() -> bool {
    return S().running;
}

auto CGpio::hasTriggerMode() -> bool {
    return S().triggerMode;
}

auto CGpio::setOutputTriggerMode(bool enable) -> bool {
    S().outTriggerMode = enable;
    return true;
}

auto CGpio::isOutputArmed() -> bool {
    return false;
}

auto CGpio::hasInputBlockMode() -> bool {
    return S().blockMode;
}

auto CGpio::setInputTriggerMode(bool enable) -> bool {
    S().inTriggerMode = enable;
    return true;
}

auto CGpio::isInputArmed() -> bool {
    return false;
}

auto CGpio::setTriggerBlocks(uint16_t blocks) -> bool {
    S().triggerBlocks = blocks;
    return true;
}

auto CGpio::getTriggerBlocks() -> uint16_t {
    return S().triggerBlocks;
}

auto CGpio::getInputTriggerCount() -> uint32_t {
    return 0;
}

auto CGpio::getInLost64(uint8_t index) -> uint64_t {
    return S().inLost ? S().inLost(index) : 0;
}

auto CGpio::getInWait64(uint8_t index) -> uint64_t {
    return S().inWait ? S().inWait(index) : 0;
}

auto CGpio::getFeatures() -> uint32_t {
    return S().features;
}

auto CGpio::hasWaitCounter() -> bool {
    return S().features & GPIO_FEATURE_WAIT;
}

// The input fills a buffer when the test says so; until then the wait takes a millisecond
auto CGpio::waitIn(uint8_t index, int) -> bool {
    auto& s = S();
    index &= 1;
    if (!s.inFull[index] && s.running && s.in && s.inFill)
        s.inFull[index] = s.inFill(index, s.inAddress[index]);
    if (!s.inFull[index])
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return s.inFull[index];
}

auto CGpio::clearInBuffer(uint8_t index) -> void {
    S().inFull[index & 1] = false;
}

auto CGpio::getInLost(uint8_t index) -> uint32_t {
    auto lost = getInLost64(index);
    return lost > UINT32_MAX ? UINT32_MAX : (uint32_t)lost;
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
