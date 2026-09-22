#include "spi_settings.h"
#include <algorithm>
#include <array>
#include "settings_json.hpp"
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
    try {
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
    } catch (...) {
        ERROR_LOG("Value %u not found in enumeration.", value)
    }
    return false;
}

auto SPIParameters::getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool {

    if (value == nullptr) {
        ERROR_LOG("Value is NULL")
        return false;
    }

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

auto SPIParameters::setDecoderSettingsString(std::string& key, std::string& value) -> bool {
    try {
        if (key == "clk") {
            m_clk = Lines::from_string(value);
            return true;
        }
        if (key == "miso") {
            m_miso = Lines::from_string(value);
            return true;
        }
        if (key == "mosi") {
            m_mosi = Lines::from_string(value);
            return true;
        }
        if (key == "cs") {
            m_cs = Lines::from_string(value);
            return true;
        }
        if (key == "cs_polarity") {
            m_cs_polarity = CsPolartiy::from_string(value);
            return true;
        }
        if (key == "bit_order") {
            m_bit_order = BitOrder::from_string(value);
            return true;
        }
        if (key == "invert_bit") {
            m_invert_bit = InvertBit::from_string(value);
            return true;
        }
    } catch (...) {
        ERROR_LOG("Value %s not found in enumeration.", value.c_str())
    }
    return false;
}

auto SPIParameters::getDecoderSettingsString(std::string& key, std::string* value) -> bool {

    if (value == nullptr) {
        ERROR_LOG("Value is NULL")
        return false;
    }

    if (key == "clk") {
        *value = m_clk.name();
        return true;
    }
    if (key == "miso") {
        *value = m_miso.name();
        return true;
    }
    if (key == "mosi") {
        *value = m_mosi.name();
        return true;
    }
    if (key == "cs") {
        *value = m_cs.name();
        return true;
    }
    if (key == "cs_polarity") {
        *value = m_cs_polarity.name();
        return true;
    }
    if (key == "bit_order") {
        *value = m_bit_order.name();
        return true;
    }
    if (key == "invert_bit") {
        *value = m_invert_bit.name();
        return true;
    }

    return false;
}

auto SPIParameters::toJson() -> std::string {
    la_json::Builder root;

    root.add("clk", m_clk.name());
    root.add("miso", m_miso.name());
    root.add("mosi", m_mosi.name());
    root.add("cs", m_cs.name());
    root.add("cpol", m_cpol);
    root.add("cpha", m_cpha);
    root.add("word_size", m_word_size);
    root.add("acq_speed", m_acq_speed);
    root.add("cs_polarity", m_cs_polarity.name());
    root.add("bit_order", m_bit_order.name());
    root.add("invert_bit", m_invert_bit.name());

    return root.str();
}

auto SPIParameters::fromJson(const std::string& json) -> bool {
    rapidjson::Document root;

    if (!la_json::parse(root, json)) {
        WARNING("Error parse json %s", json.c_str())
        return false;
    }

    try {

        auto parseUInt32 = [&](uint32_t& dest, const char* param) {
            if (!la_json::getUInt32(root, param, dest)) {
                ERROR_LOG("Missing parameter %s", param)
                return false;
            }
            return true;
        };

        std::string text;

        if (la_json::getString(root, "clk", text))
            m_clk = Lines::from_string(text);
        if (la_json::getString(root, "miso", text))
            m_miso = Lines::from_string(text);
        if (la_json::getString(root, "mosi", text))
            m_mosi = Lines::from_string(text);
        if (la_json::getString(root, "cs", text))
            m_cs = Lines::from_string(text);

        if (!parseUInt32(m_cpol, "cpol"))
            return false;
        if (!parseUInt32(m_cpha, "cpha"))
            return false;
        if (!parseUInt32(m_word_size, "word_size"))
            return false;
        if (!parseUInt32(m_acq_speed, "acq_speed"))
            return false;

        if (la_json::getString(root, "cs_polarity", text))
            m_cs_polarity = CsPolartiy::from_string(text);

        if (la_json::getString(root, "bit_order", text))
            m_bit_order = BitOrder::from_string(text);

        if (la_json::getString(root, "invert_bit", text))
            m_invert_bit = InvertBit::from_string(text);

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
