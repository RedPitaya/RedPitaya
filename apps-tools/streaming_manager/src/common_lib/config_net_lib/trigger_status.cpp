#include "trigger_status.h"
#include <sstream>
#include "json/json.h"

auto TriggerStatus::toJson() const -> std::string {
    Json::Value root;
    root["adc_armed"] = adcArmed;
    root["adc_trigger_count"] = adcTriggerCount;
    root["dac_armed"] = dacArmed;
    root["gpio_in_armed"] = gpioInArmed;
    root["gpio_out_armed"] = gpioOutArmed;
    root["gpio_in_trigger_count"] = gpioInTriggerCount;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

auto TriggerStatus::fromJson(const std::string& _json, TriggerStatus& _status) -> bool {
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::istringstream is(_json);
    if (!Json::parseFromStream(builder, is, &root, &errs) || !root.isObject())
        return false;
    try {
        // Absent keys keep their defaults, so a newer server may add fields.
        TriggerStatus status;
        if (root.isMember("adc_armed"))
            status.adcArmed = root["adc_armed"].asBool();
        if (root.isMember("adc_trigger_count"))
            status.adcTriggerCount = root["adc_trigger_count"].asUInt();
        if (root.isMember("dac_armed")) {
            const uint32_t armed = root["dac_armed"].asUInt();
            if (armed > 3)
                return false;
            status.dacArmed = static_cast<uint8_t>(armed);
        }
        if (root.isMember("gpio_in_armed"))
            status.gpioInArmed = root["gpio_in_armed"].asBool();
        if (root.isMember("gpio_out_armed"))
            status.gpioOutArmed = root["gpio_out_armed"].asBool();
        if (root.isMember("gpio_in_trigger_count"))
            status.gpioInTriggerCount = root["gpio_in_trigger_count"].asUInt();
        _status = status;
        return true;
    } catch (...) {
        return false;
    }
}
