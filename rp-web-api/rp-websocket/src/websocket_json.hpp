/**
 * $Id$
 *
 * @brief Encoding and dispatch shared by the server and the client side.
 *
 * Both ends speak the same {"key": {"type", "value"}} objects and carry the
 * same five signals, so the conversion is written once and instantiated for
 * whichever class includes it.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#pragma once
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <string>
#include <string_view>

#include "common/rp_log.h"

namespace {

/**
 * Raises the typed signal a decoded message asks for.
 *
 * Shared by the server and the client: both carry the same five signals, so
 * the dispatch is written once against whichever object is passed in.
 */
template <typename T>
void dispatch(T &target, const std::string &msg) {
    rapidjson::Document root;
    root.Parse(msg.c_str(), msg.length());
    if (root.HasParseError() || !root.IsObject()) {
        ERROR_LOG("Error parse json: %s", msg.c_str())
        return;
    }

    for (auto item = root.MemberBegin(); item != root.MemberEnd(); ++item) {
        if (!item->name.IsString() || !item->value.IsObject()) {
            continue;
        }

        const std::string key(item->name.GetString(), item->name.GetStringLength());

        auto type_field = item->value.FindMember("type");
        auto value_field = item->value.FindMember("value");
        if (value_field == item->value.MemberEnd()) {
            continue;
        }

        std::string type;
        if (type_field != item->value.MemberEnd() && type_field->value.IsString()) {
            type.assign(type_field->value.GetString(), type_field->value.GetStringLength());
        }

        const auto &value = value_field->value;

        if (type == "int" && value.IsInt()) {
            target.receiveInt(key, value.GetInt());
        } else if (type == "uint" && value.IsUint()) {
            target.receiveUInt(key, value.GetUint());
        } else if (type == "bool" && value.IsBool()) {
            target.receiveBool(key, value.GetBool());
        } else if ((type == "double" || type == "float") && value.IsNumber()) {
            // The server labels its float values "float" and a page sends
            // "double"; both are the same number on the wire, and a whole
            // number arrives without a decimal point.
            target.receiveDouble(key, value.GetDouble());
        } else if (type == "string" && value.IsString()) {
            target.receiveStr(key, std::string(value.GetString(), value.GetStringLength()));
        } else if (value.IsBool()) {
            // Older senders label booleans "string"; trust the value.
            target.receiveBool(key, value.GetBool());
        }
    }
}

/** Fills the one key object both sides exchange. */
template <typename T>
void fillObject(rapidjson::Value &object, T value, const char *type,
                rapidjson::Document::AllocatorType &allocator) {
    object.SetObject();
    object.AddMember("value", value, allocator);
    object.AddMember("type", rapidjson::StringRef(type), allocator);
}

inline void fillObject(rapidjson::Value &object, std::string_view value, const char *type,
                       rapidjson::Document::AllocatorType &allocator) {
    object.SetObject();
    object.AddMember("value",
                     rapidjson::Value(value.data(), static_cast<rapidjson::SizeType>(value.size()),
                                      allocator),
                     allocator);
    object.AddMember("type", rapidjson::StringRef(type), allocator);
}

/** Builds the whole message for one key. */
template <typename T>
std::string encode(std::string_view key, T value, const char *type) {
    rapidjson::Document document;
    document.SetObject();
    auto &allocator = document.GetAllocator();

    rapidjson::Value object;
    fillObject(object, value, type, allocator);
    document.AddMember(
        rapidjson::Value(key.data(), static_cast<rapidjson::SizeType>(key.size()), allocator),
        object, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return std::string(buffer.GetString(), buffer.GetSize());
}

/** Stores one key in a cache document, replacing what was there. */
template <typename T>
void cacheValue(rapidjson::Document &cache, std::string_view key, T value, const char *type) {
    if (!cache.IsObject()) {
        cache.SetObject();
    }
    auto &allocator = cache.GetAllocator();

    rapidjson::Value object;
    fillObject(object, value, type, allocator);

    // AddMember appends, so an existing key has to be overwritten by hand.
    auto existing = cache.FindMember(
        rapidjson::Value(key.data(), static_cast<rapidjson::SizeType>(key.size())));
    if (existing != cache.MemberEnd()) {
        existing->value = object;
        return;
    }

    cache.AddMember(
        rapidjson::Value(key.data(), static_cast<rapidjson::SizeType>(key.size()), allocator),
        object, allocator);
}

inline std::string writeDocument(const rapidjson::Document &document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return std::string(buffer.GetString(), buffer.GetSize());
}

}  // namespace
