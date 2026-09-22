#include "i2c_settings.h"
#include <algorithm>
#include <array>
#include "settings_json.hpp"
#include "rp_log.h"

using namespace i2c;

I2CParameters::I2CParameters() {
    m_scl = Lines::None;  // 0...8, 	0 if is not set
    m_sda = Lines::None;  // 0...8, 	0 if is not set
    m_acq_speed = 0;
    m_address_format = AddressFormat::Shifted;
    m_invert_bit = InvertBit::No;
}

auto I2CParameters::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
    try {
        if (key == "scl") {
            m_scl = Lines::from_int(value);
            return true;
        }
        if (key == "sda") {
            m_sda = Lines::from_int(value);
            return true;
        }
        if (key == "acq_speed") {
            m_acq_speed = value;
            return true;
        }
        if (key == "address_format") {
            m_address_format = AddressFormat::from_int(value);
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

auto I2CParameters::getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool {

    if (value == nullptr) {
        ERROR_LOG("Value is NULL")
        return false;
    }

    if (key == "scl") {
        *value = m_scl;
        return true;
    }
    if (key == "sda") {
        *value = m_sda;
        return true;
    }
    if (key == "acq_speed") {
        *value = m_acq_speed;
        return true;
    }
    if (key == "address_format") {
        *value = m_address_format;
        return true;
    }
    if (key == "invert_bit") {
        *value = m_invert_bit;
        return true;
    }
    return false;
}

auto I2CParameters::setDecoderSettingsString(std::string& key, std::string& value) -> bool {
    try {
        if (key == "scl") {
            m_scl = Lines::from_string(value);
            return true;
        }
        if (key == "sda") {
            m_sda = Lines::from_string(value);
            return true;
        }
        if (key == "address_format") {
            m_address_format = AddressFormat::from_string(value);
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

auto I2CParameters::getDecoderSettingsString(std::string& key, std::string* value) -> bool {

    if (value == nullptr) {
        ERROR_LOG("Value is NULL")
        return false;
    }

    if (key == "scl") {
        *value = m_scl.name();
        return true;
    }
    if (key == "sda") {
        *value = m_sda.name();
        return true;
    }
    if (key == "address_format") {
        *value = m_address_format.name();
        return true;
    }
    if (key == "invert_bit") {
        *value = m_invert_bit.name();
        return true;
    }
    return false;
}

auto I2CParameters::toJson() -> std::string {
    la_json::Builder root;

    root.add("scl", m_scl.name());
    root.add("sda", m_sda.name());
    root.add("acq_speed", m_acq_speed);
    root.add("address_format", m_address_format.name());
    root.add("invert_bit", m_invert_bit.name());

    return root.str();
}

auto I2CParameters::fromJson(const std::string& json) -> bool {
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

        if (la_json::getString(root, "scl", text))
            m_scl = Lines::from_string(text);
        if (la_json::getString(root, "sda", text))
            m_sda = Lines::from_string(text);
        if (!parseUInt32(m_acq_speed, "acq_speed"))
            return false;
        if (la_json::getString(root, "address_format", text))
            m_address_format = AddressFormat::from_string(text);
        if (la_json::getString(root, "invert_bit", text))
            m_invert_bit = InvertBit::from_string(text);
        return true;
    } catch (...) {
        ERROR_LOG("Error parse json. Invalid data")
        return false;
    }
    return false;
}

std::string I2CParameters::getI2CAnnotationsString(I2CAnnotations value) {
    switch (value) {
        case START:
            return "Start";
        case REPEAT_START:
            return "Repeat start";
        case STOP:
            return "Stop";

        case ACK:
            return "Ack";
        case NACK:
            return "Nack";

        case READ_ADDRESS:
            return "Read address";
        case WRITE_ADDRESS:
            return "Write address";

        case DATA_READ:
            return "Read data";
        case DATA_WRITE:
            return "Write data";
        default:
            ERROR_LOG("Unknown id = %d", (int)value)
            break;
    }
    return "";
}
