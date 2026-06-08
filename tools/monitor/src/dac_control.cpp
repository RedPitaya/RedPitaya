#include "dac_control.h"
#include <cstdio>
#include <stdexcept>
#include "ams_channels.h"
#include "ams_readers.h"
#include "rp.h"
#include "rp_session.h"

// ============================================================================
// Sets analog output voltages with bounds checking.
// ============================================================================
void set_DAC(const float* values, size_t count) {
    if (count > MAX_AO_CHANNELS) {
        throw std::out_of_range("DAC channel count exceeds maximum");
    }

    RpSession::init();

    for (size_t i = 0; i < count; ++i) {
        if (rp_AOpinSetValue(static_cast<int>(i), values[i]) != RP_OK) {
            fprintf(stderr, "Warning: Failed to set DAC channel %zu\n", i);
        }
    }
}

// ============================================================================
// Prints a formatted table of all analog monitoring system readings.
// ============================================================================
void showAMS() {
    RpSession::init();

    auto descriptions = buildDescriptions();
    float value;
    uint32_t raw;

    printf("#ID\tDesc\t\tRaw\tVal\n");

    for (size_t i = 0; i < readFunctions.size(); ++i) {
        bool ok = readFunctions[i](value, raw);
        const char* desc = descriptions[i].c_str();

        if (ok) {
            printf("%zu\t%s\t0x%08x\t%.3f\n", i, desc, raw, value);
        } else {
            printf("%zu\t%s\tERROR\tERROR\n", i, desc);
        }
    }
}

// ============================================================================
// Convenience function to release RP resources at program exit.
// ============================================================================
void cleanup() {
    RpSession::release();
}