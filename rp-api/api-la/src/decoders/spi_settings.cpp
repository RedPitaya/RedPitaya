#include "spi_settings.h"
#include <algorithm>
#include <array>
#include "json/json.h"
#include "rp_log.h"

using namespace spi;

SPIParameters::SPIParameters() {
    m_clk = Lines::None;   // 0...8, 	0 if is not set
    m_miso = Lines::None;  // 0...8,	0 if is not set
    m_mosi = Lines::None;  // 0...8,	0 if is not set
    m_cs = Lines::None;    // 0...8, 	0 if is not set
    m_cpol = 0;            // 0...1
    m_cpha = 0;            // 0...1
    m_word_size = 8;
    m_acq_speed = 0;
    m_cs_polarity = CsPolartiy::ActiveLow;
    m_bit_order = BitOrder::MsbFirst;
    m_invert_bit = InvertBit::No;
}

auto SPIParameters::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
    if (key == "clk") {
        m_clk = Lines::from_int(value);
        return true;
    }
    if (key == "miso") {
        m_miso = Lines::from_int(value);
        return true;
    }
    if (key == "mosi") {
        m_mosi = Lines::from_int(value);
        return true;
    }
    if (key == "cs") {
        m_cs = Lines::from_int(value);
        return true;
    }
    if (key == "cpol") {
        m_cpol = value;
        return true;
    }
    if (key == "cpha") {
        m_cpha = value;
        return true;
    }
    if (key == "word_size") {
        m_word_size = value;
        return true;
    }
    if (key == "acq_speed") {
        m_acq_speed = value;
        return true;
    }
    if (key == "cs_polarity") {
        m_cs_polarity = CsPolartiy::from_int(value);
        return true;
    }
    if (key == "bit_order") {
        m_bit_order = BitOrder::from_int(value);
        return true;
    }
    if (key == "invert_bit") {
        m_invert_bit = InvertBit::from_int(value);
        return true;
    }
    return false;
}

auto SPIParameters::getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool {
    if (key == "clk") {
        *value = m_clk;
        return true;
    }
    if (key == "miso") {
        *value = m_miso;
        return true;
    }
    if (key == "mosi") {
        *value = m_mosi;
        return true;
    }
    if (key == "cs") {
        *value = m_cs;
        return true;
    }
    if (key == "cpol") {
        *value = m_cpol;
        return true;
    }
    if (key == "cpha") {
        *value = m_cpha;
        return true;
    }
    if (key == "word_size") {
        *value = m_word_size;
        return true;
    }
    if (key == "acq_speed") {
        *value = m_acq_speed;
        return true;
    }
    if (key == "cs_polarity") {
        *value = m_cs_polarity;
        return true;
    }
    if (key == "bit_order") {
        *value = m_bit_order;
        return true;
    }
    if (key == "invert_bit") {
        *value = m_invert_bit;
        return true;
    }
    return false;
}

auto SPIParameters::toJson() -> std::string {
    Json::Value root;

    root["clk"] = m_clk.name();
    root["miso"] = m_miso.name();
    root["mosi"] = m_mosi.name();
    root["cs"] = m_cs.name();
    root["cpol"] = m_cpol;
    root["cpha"] = m_cpha;
    root["word_size"] = m_word_size;
    root["acq_speed"] = m_acq_speed;
    root["cs_polarity"] = m_cs_polarity.name();
    root["bit_order"] = m_bit_order.name();
    root["invert_bit"] = m_invert_bit.name();

    Json::StreamWriterBuilder builder;
    const std::string json = Json::writeString(builder, root);
    return json;
}

auto SPIParameters::fromJson(const std::string& json) -> bool {
    Json::Value root;

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    auto is = std::istringstream(json);
    if (!parseFromStream(builder, is, &root, &errs)) {
        WARNING("Error parse json %s", errs.c_str())
        return false;
    }

    try {

        auto parseUInt32 = [&](uint32_t& dest, std::string param) {
            if (root.isMember(param)) {
                dest = root[param].asUInt();
            } else {
                ERROR_LOG("Missing parameter %s", param.c_str())
                return false;
            }
            return true;
        };

        if (root.isMember("clk"))
            m_clk = Lines::from_string(root["clk"].asString());
        if (root.isMember("miso"))
            m_miso = Lines::from_string(root["miso"].asString());
        if (root.isMember("mosi"))
            m_mosi = Lines::from_string(root["mosi"].asString());
        if (root.isMember("cs"))
            m_cs = Lines::from_string(root["cs"].asString());

        if (!parseUInt32(m_cpol, "cpol"))
            return false;
        if (!parseUInt32(m_cpha, "cpha"))
            return false;
        if (!parseUInt32(m_word_size, "word_size"))
            return false;
        if (!parseUInt32(m_acq_speed, "acq_speed"))
            return false;

        if (root.isMember("cs_polarity"))
            m_cs_polarity = CsPolartiy::from_string(root["cs_polarity"].asString());

        if (root.isMember("bit_order"))
            m_bit_order = BitOrder::from_string(root["bit_order"].asString());

        if (root.isMember("invert_bit"))
            m_invert_bit = InvertBit::from_string(root["invert_bit"].asString());

        return true;
    } catch (...) {
        ERROR_LOG("Error parse json. Invalid data")
        return false;
    }
    return false;
}

std::string SPIParameters::getSPIAnnotationsString(SPIAnnotations value) {
    switch (value) {
        case DATA:
            return "Data";
        default:
            TRACE_SHORT("Unknown id = %d", (int)value)
            break;
    }
    return "";
}
