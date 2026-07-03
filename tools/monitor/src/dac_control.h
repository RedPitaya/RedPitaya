#ifndef DAC_CONTROL_H
#define DAC_CONTROL_H

#include <cstddef>

// ============================================================================
// Sets analog output voltages with bounds checking.
// Throws if count exceeds hardware limits.
// ============================================================================
void set_DAC(const float* values, size_t count);

// ============================================================================
// Prints a formatted table of all analog monitoring system readings.
// Each row contains: channel ID, description, raw register value, and float value.
// ============================================================================
void showAMS();

// ============================================================================
// Convenience function to release RP resources at program exit.
// Safe to call even if RpSession was never initialized.
// ============================================================================
void cleanup();

#endif  // DAC_CONTROL_H