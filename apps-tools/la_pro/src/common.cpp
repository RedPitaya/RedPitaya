#include "common.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "rp.h"
#include "rp_hw-profiles.h"

auto getMAXFreq() -> uint32_t {
    uint32_t value = 125e6;
    if (rp_HPGetBaseSpeedHz(&value) != RP_HP_OK) {
        ERROR_LOG("Can't get base speed");
    }
    return value;
}

/** Parses a configuration object, empty input and malformed text both fail. */
static auto parseConfig(rapidjson::Document& document, const std::string& json) -> bool {
    if (json.length() == 0) {
        return false;
    }

    document.Parse(json.c_str(), json.length());
    if (document.HasParseError() || !document.IsObject()) {
        ERROR_LOG("Error parse json: %s", json.c_str())
        return false;
    }
    return true;
}

static auto writeValue(const rapidjson::Value& value) -> std::string {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return std::string(buffer.GetString(), buffer.GetSize());
}

auto getNameFromConfig(std::string& json) -> std::string {
    rapidjson::Document root;
    if (!parseConfig(root, json)) {
        return "";
    }

    auto name = root.FindMember("name");
    if (name == root.MemberEnd() || !name->value.IsString()) {
        return "";
    }
    return std::string(name->value.GetString(), name->value.GetStringLength());
}

auto getParamFromConfig(std::string& json) -> std::string {
    rapidjson::Document root;
    if (!parseConfig(root, json)) {
        return "";
    }

    auto config = root.FindMember("config");
    if (config == root.MemberEnd()) {
        return "";
    }
    return writeValue(config->value);
}

auto getConfig(std::string& name, std::string& param_json) -> std::string {
    rapidjson::Document root;
    if (!parseConfig(root, param_json)) {
        return "";
    }

    rapidjson::Document data;
    data.SetObject();
    auto& allocator = data.GetAllocator();

    data.AddMember("name",
                   rapidjson::Value(name.c_str(), static_cast<rapidjson::SizeType>(name.size()),
                                    allocator),
                   allocator);
    // The parsed object is moved in, so it keeps its own allocator alive
    // through the copy rather than referencing freed storage.
    data.AddMember("config", rapidjson::Value(root, allocator), allocator);
    return writeValue(data);
}

auto annoToJSON(std::map<uint8_t, std::string> map) -> std::string {
    rapidjson::Document root;
    root.SetObject();
    auto& allocator = root.GetAllocator();

    for (const auto& itm : map) {
        const std::string key = std::to_string(itm.first);
        root.AddMember(
            rapidjson::Value(key.c_str(), static_cast<rapidjson::SizeType>(key.size()), allocator),
            rapidjson::Value(itm.second.c_str(),
                             static_cast<rapidjson::SizeType>(itm.second.size()), allocator),
            allocator);
    }
    return writeValue(root);
}

auto getModel() -> rp_HPeModels_t {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }
    return c;
}
