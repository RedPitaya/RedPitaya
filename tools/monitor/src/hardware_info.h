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
    // Returns "35" for 250-12 series, "5" for all others
    static const std::string& getDDRVoltage();

    // Checks if the board belongs to the 250-12 family
    static bool is250_12_series();

   private:
    // Lazily computed DDR voltage string
    static std::string queryDDRVoltage();

    // Reads the hardware model from the device
    static bool queryIs250_12();
};

#endif  // HARDWARE_INFO_H