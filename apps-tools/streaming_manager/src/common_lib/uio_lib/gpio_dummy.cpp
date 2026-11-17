#include <chrono>
#include <mutex>
#include <new>
#include <thread>

#include "gpio.h"
#include "gpio_dummy.h"

using namespace uio_lib;

auto uio_lib::dummy::gpioState() -> GpioState& {
    static GpioState state;
    return state;
}

auto uio_lib::dummy::resetGpioState() -> void {
    // the atomics cannot be assigned: made anew in place
    auto& s = gpioState();
    s.~GpioState();
    new (&s) GpioState();
}

namespace {

auto S() -> dummy::GpioState& {
    return dummy::gpioState();
}

}  // namespace

auto CGpio::create(const std::vector<UioT>&) -> Ptr {
    return S().present ? std::make_shared<CGpio>(0, nullptr, 0) : Ptr();
}

CGpio::CGpio(int _fd, void* _regset, size_t _regsetSize) : m_Fd(_fd), m_Regset(_regset), m_RegsetSize(_regsetSize), m_Map(nullptr), m_lock(), m_inRunning(false), m_outRunning(false), m_eventSource(gpio_event_id) {}

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
    std::lock_guard lock(m_lock);
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

auto CGpio::prepareInput() -> void {
    stopInput();
    std::lock_guard lock(m_lock);
    S().inPrepares++;
    S().inFull = {false, false};
}

auto CGpio::prepareOutput() -> void {
    stopOutput();
    std::lock_guard lock(m_lock);
    S().outPrepares++;
}

auto CGpio::startInput() -> void {
    std::lock_guard lock(m_lock);
    S().inRunning = true;
    S().inStarts++;
}

auto CGpio::startOutput() -> void {
    std::lock_guard lock(m_lock);
    S().outRunning = true;
    S().outStarts++;
}

auto CGpio::stopInput() -> void {
    std::lock_guard lock(m_lock);
    S().inRunning = false;
    S().inStops++;
}

auto CGpio::stopOutput() -> void {
    std::lock_guard lock(m_lock);
    S().outRunning = false;
    S().outStops++;
}

auto CGpio::prepare(bool in, bool out) -> void {
    if (in)
        prepareInput();
    if (out)
        prepareOutput();
}

auto CGpio::start(bool in, bool out) -> void {
    if (out)
        startOutput();
    if (in)
        startInput();
}

auto CGpio::stop() -> void {
    stopInput();
    stopOutput();
}

auto CGpio::softwareTrigger() -> void {
    std::lock_guard lock(m_lock);
    S().softwareTriggers++;
}

auto CGpio::isRunning() -> bool {
    std::lock_guard lock(m_lock);
    return S().inRunning || S().outRunning;
}

auto CGpio::isInputRunning() -> bool {
    std::lock_guard lock(m_lock);
    return S().inRunning;
}

auto CGpio::isOutputRunning() -> bool {
    std::lock_guard lock(m_lock);
    return S().outRunning;
}

auto CGpio::getOutWait64() -> uint64_t {
    return S().outWait ? S().outWait() : 0;
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

// The input fills a buffer when the test says so; until then the wait takes a millisecond.
// The state is shared with the threads that start and stop the block: under m_lock, as the
// registers of the real one; the fill of the test and the sleep run unlocked.
auto CGpio::waitIn(uint8_t index, int) -> bool {
    auto& s = S();
    index &= 1;
    bool full = false;
    bool running = false;
    uint32_t address = 0;
    {
        std::lock_guard lock(m_lock);
        full = s.inFull[index];
        running = s.inRunning;
        address = s.inAddress[index];
    }
    if (!full && running && s.inFill) {
        full = s.inFill(index, address);
        std::lock_guard lock(m_lock);
        s.inFull[index] = full;
    }
    if (!full)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return full;
}

auto CGpio::clearInBuffer(uint8_t index) -> void {
    std::lock_guard lock(m_lock);
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
