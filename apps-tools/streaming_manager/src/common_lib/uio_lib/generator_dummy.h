#ifndef UIO_LIB_GENERATOR_DUMMY_H
#define UIO_LIB_GENERATOR_DUMMY_H

#include <cstdint>
#include <functional>

namespace uio_lib::dummy {

// Host builds: what the dummy CGenerator reports, set by the tests, and what it was given.
// Defaults: every buffer is taken at once, no loop mode.
struct GenState {
    // reported
    std::function<bool(uint8_t index)> slotFree;  // setDataAddress() may take a buffer into the slot

    // given
    std::function<void(uint8_t index, uint32_t ch1, uint32_t ch2, uint32_t size)> taken;  // a buffer taken into a slot
    bool running = false;
    int starts = 0;
    int stops = 0;
};

auto genState() -> GenState&;
auto resetGenState() -> void;

}  // namespace uio_lib::dummy

#endif
