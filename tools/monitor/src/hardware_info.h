#ifndef HARDWARE_INFO_H
#define HARDWARE_INFO_H

#include <string>
#include "rp_hw-profiles.h"
#include "rp_hw.h"

// ============================================================================
// Cached hardware information queries.
// Board model and DDR voltage are queried once and reused.
// ============================================================================
class HardwareInfo {
   public:
    static const std::string& getDDRVoltage();

    static bool isLowVRAM_series();

   private:
    // Lazily computed DDR voltage string
    static std::string queryDDRVoltage();

    // Reads the hardware model from the device
    static bool queryIsLowVRAM();
};

#endif  // HARDWARE_INFO_H