#ifndef UIO_LIB_GPIO_DUMMY_H
#define UIO_LIB_GPIO_DUMMY_H

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>

namespace uio_lib::dummy {

// Host builds: the dummy CGpio, set by the tests, and what it was given. Without `present` there
// is no GPIO block (CGpio::create gives none), as before.
struct GpioState {
    bool present = false;
    bool triggerMode = false;  // the output trigger mode register
    bool blockMode = false;    // the input trigger and block mode
    uint32_t features = 0;

    // reported
    // fills input buffer `index` at `address`; true: the buffer is full (else waitIn waits 1 ms)
    std::function<bool(uint8_t index, uint32_t address)> inFill;
    std::function<uint64_t(uint8_t index)> inLost;
    std::function<uint64_t(uint8_t index)> inWait;
    std::function<bool(uint8_t index)> outDone;  // output buffer `index` played, default true
    std::function<uint64_t()> outWait;            // getOutWait64()

    // given
    std::function<void(uint8_t index, uint32_t address)> outTaken;  // an output buffer handed over
    uint8_t outputsP = 0;
    uint8_t outputsN = 0;
    uint16_t polarity = 0;
    uint32_t decimation = 0;
    bool rle = false;
    uint32_t outStep = 0;
    uint32_t dataSize = 0;
    uint32_t triggerSources = 0;
    bool inTriggerMode = false;
    bool outTriggerMode = false;
    uint16_t triggerBlocks = 0;
    std::array<uint32_t, 2> inAddress{};
    std::array<bool, 2> inFull{};
    // set by the threads of the servers, read by the tests while they run
    std::atomic<bool> inRunning{false};
    std::atomic<bool> outRunning{false};
    std::atomic<int> inPrepares{0};
    std::atomic<int> outPrepares{0};
    std::atomic<int> inStarts{0};
    std::atomic<int> outStarts{0};
    std::atomic<int> inStops{0};
    std::atomic<int> outStops{0};
    std::atomic<int> softwareTriggers{0};
};

auto gpioState() -> GpioState&;
// Between the tests, with no server running.
auto resetGpioState() -> void;

}  // namespace uio_lib::dummy

#endif
