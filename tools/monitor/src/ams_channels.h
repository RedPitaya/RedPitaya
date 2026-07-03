#ifndef AMS_CHANNELS_H
#define AMS_CHANNELS_H

#include <array>
#include <cstdint>
#include <string>

// ============================================================================
// Strongly-typed enumeration for all available analog measurement channels
// ============================================================================
enum class AmsChannel : int {
    Temp = 0,
    AI0,
    AI1,
    AI2,
    AI3,
    AI4_5V,
    VCCPINT,
    VCCPAUX,
    VCCBRAM,
    VCCINT,
    VCCAUX,
    VCCDDR,
    AO0,
    AO1,
    AO2,
    AO3,
    Count  // Sentinel value representing the number of channels
};

// ============================================================================
// Result structure for a single AMS reading
// ============================================================================
struct AmsResult {
    float value;              // Floating-point voltage or temperature
    uint32_t raw;             // Raw ADC register value
    const char* description;  // Human-readable label
};

// ============================================================================
// Type alias for channel reader functions.
// Returns true on success, populates value and raw on output.
// ============================================================================
using ReadFunc = bool (*)(float&, uint32_t&);

// ============================================================================
// Channel capacity constants
// ============================================================================
constexpr size_t MAX_AO_CHANNELS = 4;  // Maximum analog output channels
constexpr size_t MAX_AI_CHANNELS = 4;  // Maximum analog input channels

#endif  // AMS_CHANNELS_H