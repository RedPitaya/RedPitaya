#include "record_status.h"
#include <sstream>
#include "json/json.h"

namespace {

auto toValue(const StreamRecordStatus& s) -> Json::Value {
    Json::Value v;
    v["mode"] = s.mode;
    v["running"] = s.running;
    v["samples"] = Json::UInt64(s.samples);
    v["limit"] = Json::UInt64(s.limit);
    v["written"] = Json::UInt64(s.written);
    v["write_rate"] = s.writeRate;
    v["queue"] = s.queue;
    v["lost_board"] = Json::UInt64(s.lostBoard);
    v["lost_queue"] = Json::UInt64(s.lostQueue);
    v["file"] = s.file;
    return v;
}

auto fromValue(const Json::Value& v, StreamRecordStatus& s) -> bool {
    if (!v.isObject())
        return false;
    if (v.isMember("mode")) {
        const uint32_t m = v["mode"].asUInt();
        if (m > StreamRecordStatus::SD)
            return false;
        s.mode = static_cast<uint8_t>(m);
    }
    if (v.isMember("running"))
        s.running = v["running"].asBool();
    if (v.isMember("samples"))
        s.samples = v["samples"].asUInt64();
    if (v.isMember("limit"))
        s.limit = v["limit"].asUInt64();
    if (v.isMember("written"))
        s.written = v["written"].asUInt64();
    if (v.isMember("write_rate"))
        s.writeRate = v["write_rate"].asDouble();
    if (v.isMember("queue"))
        s.queue = v["queue"].asDouble();
    if (v.isMember("lost_board"))
        s.lostBoard = v["lost_board"].asUInt64();
    if (v.isMember("lost_queue"))
        s.lostQueue = v["lost_queue"].asUInt64();
    if (v.isMember("file"))
        s.file = v["file"].asString();
    return true;
}

}  // namespace

auto RecordStatus::toJson() const -> std::string {
    Json::Value root;
    root["adc"] = toValue(adc);
    root["gpio_in"] = toValue(gpioIn);
    root["sd_free"] = Json::UInt64(sdFree);
    root["sd_total"] = Json::UInt64(sdTotal);
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

auto RecordStatus::fromJson(const std::string& _json, RecordStatus& _status) -> bool {
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::istringstream is(_json);
    if (!Json::parseFromStream(builder, is, &root, &errs) || !root.isObject())
        return false;
    try {
        RecordStatus status;
        if (root.isMember("adc") && !fromValue(root["adc"], status.adc))
            return false;
        if (root.isMember("gpio_in") && !fromValue(root["gpio_in"], status.gpioIn))
            return false;
        if (root.isMember("sd_free"))
            status.sdFree = root["sd_free"].asUInt64();
        if (root.isMember("sd_total"))
            status.sdTotal = root["sd_total"].asUInt64();
        _status = status;
        return true;
    } catch (...) {
        return false;
    }
}
