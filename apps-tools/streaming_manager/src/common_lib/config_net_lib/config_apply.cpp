#include "config_apply.h"
#include <sstream>
#include "json/json.h"

namespace {

auto parse(const std::string& _json, Json::Value& _root) -> bool {
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::istringstream is(_json);
    return Json::parseFromStream(builder, is, &_root, &errs) && _root.isObject();
}

// Numbers are compared by value: the document may carry 8 where the settings write 8u.
auto sameValue(const Json::Value& a, const Json::Value& b) -> bool {
    if (a.isString() && b.isString())
        return a.asString() == b.asString();
    if (a.isBool() && b.isBool())
        return a.asBool() == b.asBool();
    if (a.isNumeric() && b.isNumeric()) {
        if (a.isInt64() && b.isInt64())
            return a.asInt64() == b.asInt64();
        if (a.isUInt64() && b.isUInt64())
            return a.asUInt64() == b.asUInt64();
    }
    return false;
}

}  // namespace

auto applyConfigJson(const std::string& _json, CStreamSettings& _settings) -> bool {
    CStreamSettings result(_settings);
    if (!result.parseJson(_json))
        return false;
    Json::Value incoming;
    Json::Value applied;
    if (!parse(_json, incoming) || !parse(result.toJson(), applied))
        return false;
    for (const auto& section : incoming.getMemberNames()) {
        if (!applied.isMember(section) || !incoming[section].isObject())
            continue;
        for (const auto& key : incoming[section].getMemberNames()) {
            if (applied[section].isMember(key) && !sameValue(incoming[section][key], applied[section][key]))
                return false;
        }
    }
    _settings = result;
    return true;
}
