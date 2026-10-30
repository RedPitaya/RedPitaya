#ifndef UIO_LIB_OSCILLOSCOPE_DUMMY_H
#define UIO_LIB_OSCILLOSCOPE_DUMMY_H

#include <array>
#include <cstdint>
#include <functional>

namespace uio_lib::dummy {

// Host builds: what the dummy COscilloscope reports, set by the tests, and what it was given.
// Defaults: a buffer is full at once, nothing lost, no wait counter, no block mode.
struct OscState {
    // reported
    std::function<bool()> wait;                                // a DMA buffer is full
    std::function<uint64_t(uint8_t index, uint8_t ch)> lost;  // getBufferLost()
    std::function<uint64_t(uint8_t index)> waitCount;         // getBufferWait()
    std::function<uint32_t()> triggerCount;
    std::function<int64_t(uint8_t index)> time;               // getBufferTime()
    bool waitCounter = false;
    bool blockMode = false;
    bool armed = false;

    // given
    struct Calibration {
        int32_t offset = 0;
        float gain = 1;
        bool beforeFilter = false;
        int32_t aa = 0, bb = 0, kk = 0, pp = 0;
    };
    uint32_t decimation = 0;
    bool eightBit = false;
    bool filterBypass = true;
    std::array<Calibration, 4> calibration{};
    uint32_t dataSize = 0;
    bool triggerMode = false;
    uint32_t triggerSources = 0;
    int32_t triggerLow = 0;
    int32_t triggerHigh = 0;
    bool triggerFalling = false;
    uint32_t eventSource = 0;
    uint16_t triggerBlocks = 0;
    bool running = false;
    int prepares = 0;
    int stops = 0;
    int softwareTriggers = 0;
};

auto oscState() -> OscState&;
auto resetOscState() -> void;

}  // namespace uio_lib::dummy

#endif
