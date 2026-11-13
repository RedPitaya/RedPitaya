#include "board_info.h"
#include <sstream>
#include "json/json.h"

auto BoardInfoMessage::toJson() const -> std::string {
    Json::Value root;
    root["model"] = model;
    root["mac"] = mac;
    root["master"] = master;
    root["adc_channels"] = adcChannels;
    root["dac_channels"] = dacChannels;
    root["adc_rate"] = adcRate;
    root["dac_rate"] = dacRate;
    root["adc_bits"] = adcBits;
    root["adc_ranges"] = adcRanges;
    root["adc_ac_dc"] = adcACDC;
    root["dac_gain"] = dacGain;
    root["adc"] = adc;
    root["adc_block_mode"] = adcBlockMode;
    root["dac"] = dac;
    root["dac_trigger_outputs"] = dacTriggerOutputs;
    root["gpio"] = gpio;
    root["ext_trigger"] = extTrigger;
    root["memory_size"] = memorySize;
    root["memory_max_block"] = memoryMaxBlock;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

auto BoardInfoMessage::fromJson(const std::string& _json, BoardInfoMessage& _info) -> bool {
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::istringstream is(_json);
    if (!Json::parseFromStream(builder, is, &root, &errs) || !root.isObject())
        return false;
    try {
        // Absent keys keep their defaults, so a newer server may add fields.
        BoardInfoMessage info;
        auto u8 = [&](const char* key, uint8_t& value, uint32_t max) -> bool {
            if (!root.isMember(key))
                return true;
            const uint32_t v = root[key].asUInt();
            if (v > max)
                return false;
            value = static_cast<uint8_t>(v);
            return true;
        };
        auto flag = [&](const char* key, bool& value) {
            if (root.isMember(key))
                value = root[key].asBool();
        };
        if (!u8("model", info.model, 255) || !u8("adc_channels", info.adcChannels, 4) || !u8("dac_channels", info.dacChannels, 2) ||
            !u8("adc_bits", info.adcBits, 16))
            return false;
        if (root.isMember("mac"))
            info.mac = root["mac"].asString();
        if (root.isMember("adc_rate"))
            info.adcRate = root["adc_rate"].asUInt();
        if (root.isMember("dac_rate"))
            info.dacRate = root["dac_rate"].asUInt();
        if (root.isMember("memory_size"))
            info.memorySize = root["memory_size"].asUInt();
        if (root.isMember("memory_max_block"))
            info.memoryMaxBlock = root["memory_max_block"].asUInt();
        flag("master", info.master);
        flag("adc_ranges", info.adcRanges);
        flag("adc_ac_dc", info.adcACDC);
        flag("dac_gain", info.dacGain);
        flag("adc", info.adc);
        flag("adc_block_mode", info.adcBlockMode);
        flag("dac", info.dac);
        flag("dac_trigger_outputs", info.dacTriggerOutputs);
        flag("gpio", info.gpio);
        flag("ext_trigger", info.extTrigger);
        _info = info;
        return true;
    } catch (...) {
        return false;
    }
}
