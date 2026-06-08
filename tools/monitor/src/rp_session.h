#ifndef RP_SESSION_H
#define RP_SESSION_H

#include <stdexcept>
#include "rp.h"

// ============================================================================
// RAII-based lifecycle manager for the Red Pitaya library.
// Ensures initialization happens only once and cleanup is controlled.
// Thread-safe in C++17 and later (inline static).
// ============================================================================
class RpSession {
   public:
    // Initialize the RP library if not already initialized
    static void init() {
        if (!initialized_) {
            if (rp_InitReset(false) != RP_OK) {
                throw std::runtime_error("Failed to initialize Red Pitaya");
            }
            initialized_ = true;
        }
    }

    // Release RP resources and allow re-initialization later
    static void release() {
        if (initialized_) {
            rp_Release();
            initialized_ = false;
        }
    }

    // Check if the library is currently initialized
    static bool isInitialized() { return initialized_; }

   private:
    static inline bool initialized_ = false;
};

#endif  // RP_SESSION_H