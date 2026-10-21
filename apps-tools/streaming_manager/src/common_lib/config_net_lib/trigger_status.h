#ifndef CONFIG_NET_LIB_TRIGGER_STATUS_H
#define CONFIG_NET_LIB_TRIGGER_STATUS_H

#include <cstdint>
#include <string>

// Trigger state of the stream_app blocks, sent with CS_RESPONSE_TRIGGER_STATUS.
// The buffer losses are not here: they travel with the data in the buffer headers.
struct TriggerStatus {
    bool adcArmed = false;
    uint32_t adcTriggerCount = 0;  // triggers seen in block mode
    uint8_t dacArmed = 0;          // bit 0 - channel 1, bit 1 - channel 2
    bool gpioInArmed = false;
    bool gpioOutArmed = false;
    uint32_t gpioInTriggerCount = 0;

    auto toJson() const -> std::string;
    // False on a document that is not a status (including an empty one); _status is then unchanged.
    static auto fromJson(const std::string& _json, TriggerStatus& _status) -> bool;

    auto operator==(const TriggerStatus&) const -> bool = default;
};

#endif
