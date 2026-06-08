#ifndef AMS_READERS_H
#define AMS_READERS_H

#include <array>
#include "ams_channels.h"

// ============================================================================
// Dispatch table of channel reader functions.
// Maps each AmsChannel to its corresponding reader function.
// Index order matches the AmsChannel enum.
// ============================================================================
extern const std::array<ReadFunc, static_cast<size_t>(AmsChannel::Count)> readFunctions;

// ============================================================================
// Builds human-readable descriptions for all channels.
// The DDR voltage string is injected dynamically based on the board model.
// ============================================================================
std::array<std::string, static_cast<size_t>(AmsChannel::Count)> buildDescriptions();

#endif  // AMS_READERS_H