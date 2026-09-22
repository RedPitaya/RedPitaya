/**
 * $Id$
 *
 * @brief JSON helpers shared by the decoder settings.
 *
 * The settings are flat objects of strings and unsigned numbers, so the
 * decoders only need these few calls instead of the parser's own API.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#pragma once

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cstdint>
#include <string>

namespace la_json {

/** Collects the members of one settings object and writes it out. */
class Builder {
   public:
    Builder() { m_document.SetObject(); }

    auto add(const char *name, const std::string &value) -> void {
        auto &allocator = m_document.GetAllocator();
        m_document.AddMember(
            rapidjson::Value(rapidjson::StringRef(name)),
            rapidjson::Value(value.c_str(), static_cast<rapidjson::SizeType>(value.size()),
                             allocator),
            allocator);
    }

    auto add(const char *name, float value) -> void {
        auto &allocator = m_document.GetAllocator();
        m_document.AddMember(rapidjson::Value(rapidjson::StringRef(name)),
                             rapidjson::Value(value), allocator);
    }

    auto add(const char *name, uint32_t value) -> void {
        auto &allocator = m_document.GetAllocator();
        m_document.AddMember(rapidjson::Value(rapidjson::StringRef(name)),
                             rapidjson::Value(value), allocator);
    }

    auto str() const -> std::string {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        m_document.Accept(writer);
        return std::string(buffer.GetString(), buffer.GetSize());
    }

   private:
    rapidjson::Document m_document;
};

/** Parses a settings object, false on malformed input. */
inline auto parse(rapidjson::Document &document, const std::string &json) -> bool {
    document.Parse(json.c_str(), json.length());
    return !document.HasParseError() && document.IsObject();
}

inline auto getString(const rapidjson::Document &document, const char *name, std::string &out)
    -> bool {
    auto member = document.FindMember(name);
    if (member == document.MemberEnd() || !member->value.IsString()) {
        return false;
    }
    out.assign(member->value.GetString(), member->value.GetStringLength());
    return true;
}

/** Accepts the number however it was written, as long as it is not negative. */
inline auto getUInt32(const rapidjson::Document &document, const char *name, uint32_t &out)
    -> bool {
    auto member = document.FindMember(name);
    if (member == document.MemberEnd() || !member->value.IsNumber()) {
        return false;
    }

    if (member->value.IsUint()) {
        out = member->value.GetUint();
        return true;
    }

    const double value = member->value.GetDouble();
    if (value < 0.0 || value > static_cast<double>(UINT32_MAX)) {
        return false;
    }
    out = static_cast<uint32_t>(value);
    return true;
}

inline auto getFloat(const rapidjson::Document &document, const char *name, float &out) -> bool {
    auto member = document.FindMember(name);
    if (member == document.MemberEnd() || !member->value.IsNumber()) {
        return false;
    }
    out = static_cast<float>(member->value.GetDouble());
    return true;
}

}  // namespace la_json
