#ifndef UIO_LIB_TRIGGER_H
#define UIO_LIB_TRIGGER_H

#include <cstdint>

namespace uio_lib {

// Trigger sources of the stream_app trigger masks (scope 0x08, DAC 0x24, GPIO 0x84).
// The bits follow rp_concat: {ext, la, osc2, osc1, gen2, gen1}. The 4-input board has
// no DAC; its BD wires the triggers of ADC CH3 and CH4 to the generator bits.
constexpr uint32_t TRIG_SRC_GEN_A = 1 << 0;
constexpr uint32_t TRIG_SRC_GEN_B = 1 << 1;
constexpr uint32_t TRIG_SRC_ADC_CH1 = 1 << 2;
constexpr uint32_t TRIG_SRC_ADC_CH2 = 1 << 3;
constexpr uint32_t TRIG_SRC_LA = 1 << 4;
constexpr uint32_t TRIG_SRC_EXT = 1 << 5;
constexpr uint32_t TRIG_SRC_ALL = 0x3F;

// Event sources for the event selectors (index into the rp_concat event vectors).
constexpr uint32_t EVENT_SRC_GEN_A = 0;
constexpr uint32_t EVENT_SRC_GEN_B = 1;
constexpr uint32_t EVENT_SRC_OSC1 = 2;
constexpr uint32_t EVENT_SRC_OSC2 = 3;
constexpr uint32_t EVENT_SRC_LA = 4;
constexpr uint32_t EVENT_SRC_MAX = 4;

// Event register bits of the scope (0x00) and the DAC (0x1C).
// The GPIO event register (0x88) uses the reverse order, see gpio.h.
constexpr uint32_t EVENT_RESET = 1 << 0;
constexpr uint32_t EVENT_START = 1 << 1;
constexpr uint32_t EVENT_STOP = 1 << 2;
constexpr uint32_t EVENT_TRIG = 1 << 3;

// Mask bit of the level trigger of ADC channel `ch` (0-based), 0 if the board has no such channel.
inline auto adcTriggerSource(uint8_t ch, uint8_t maxChannels) -> uint32_t {
    if (ch >= maxChannels)
        return 0;
    switch (ch) {
        case 0:
            return TRIG_SRC_ADC_CH1;
        case 1:
            return TRIG_SRC_ADC_CH2;
        case 2:
            return TRIG_SRC_GEN_A;
        case 3:
            return TRIG_SRC_GEN_B;
        default:
            return 0;
    }
}

}  // namespace uio_lib

#endif
