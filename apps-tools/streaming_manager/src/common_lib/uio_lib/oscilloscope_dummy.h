#ifndef UIO_LIB_OSCILLOSCOPE_DUMMY_H
#define UIO_LIB_OSCILLOSCOPE_DUMMY_H

#include <cstdint>
#include <functional>

namespace uio_lib::dummy {

// Host builds: what the dummy COscilloscope reports, set by the tests. Defaults: a buffer is
// full at once, nothing lost, no trigger mode, no wait counter.
struct OscState {
    std::function<bool()> wait;                                // a DMA buffer is full
    std::function<uint64_t(uint8_t index, uint8_t ch)> lost;  // getBufferLost()
    std::function<uint64_t(uint8_t index)> waitCount;         // getBufferWait()
    std::function<uint32_t()> triggerCount;
    bool triggerMode = false;
    bool waitCounter = false;
    uint16_t triggerBlocks = 0;
};

auto oscState() -> OscState&;
auto resetOscState() -> void;

}  // namespace uio_lib::dummy

#endif
