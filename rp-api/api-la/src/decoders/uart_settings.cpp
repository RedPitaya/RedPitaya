#include "uart_settings.h"
#include <algorithm>
#include <array>
#include "settings_json.hpp"
#include "rp_log.h"

using namespace uart;

UARTParameters::UARTParameters() {
    m_rx = Lines::None;  // 1...8, 0 if is not set
    m_tx = Lines::None;  // 1...8, 0 if is not set
    m_baudrate = 9600;
    m_invert = InvertBit::No;
    m_bitOrder = UartBitOrder::LsbFirst;
    m_num_data_bits = NumDataBits::Bits8;
    m_parity = Parity::None;
    m_num_stop_bits = NumStopBits::Stop_Bit_10;
    m_samplerate = 0;
}

auto UARTParameters::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
    try {
        if (key == "rx") {
            m_rx = Lines::from_int(value);
            return true;
        }
        if (key == "tx") {
            m_tx = Lines::from_int(value);
            return true;
        }
        if (key == "baudrate") {
            m_baudrate = value;
            return true;
        }
        if (key == "invert") {
            m_invert = InvertBit::from_int(value);
            return true;
        }
        if (key == "bitOrder") {
            m_bitOrder = UartBitOrder::from_int(value);
            return true;
        }
        if (key == "num_data_bits") {
            m_num_data_bits = NumDataBits::from_int(value);
            return true;
        }
        if (key == "parity") {
            m_parity = Parity::from_int(value);
            return true;
        }
        if (key == "num_stop_bits") {
            m_num_stop_bits = NumStopBits::from_int(value);
            return true;
        }
        if (key == "acq_speed") {
            m_samplerate = value;
            return true;
        }
    } catch (...) {
        ERROR_LOG("Value %u not found in enumeration.", value)
    }
    return false;
}

auto UARTParameters::getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool {

    if (value == nullptr) {
        ERROR_LOG("Value is NULL")
        return false;
    }

    if (key == "rx") {
        *value = m_rx;
        return true;
    }
    if (key == "tx") {
        *value = m_tx;
        return true;
    }
    if (key == "baudrate") {
        *value = m_baudrate;
        return true;
    }
    if (key == "invert") {
        *value = m_invert;
        return true;
    }
    if (key == "bitOrder") {
        *value = m_bitOrder;
        return true;
    }
    if (key == "num_data_bits") {
        *value = m_num_data_bits;
        return true;
    }
    if (key == "parity") {
        *value = m_parity;
        return true;
    }
    if (key == "num_stop_bits") {
        *value = m_num_stop_bits;
        return true;
    }
    if (key == "acq_speed") {
        *value = m_samplerate;
        return true;
    }
    return false;
}

auto UARTParameters::setDecoderSettingsString(std::string& key, std::string& value) -> bool {
    try {
        if (key == "rx") {
            m_rx = Lines::from_string(value);
            return true;
        }
        if (key == "tx") {
            m_tx = Lines::from_string(value);
            return true;
        }
        if (key == "invert") {
            m_invert = InvertBit::from_string(value);
            return true;
        }
        if (key == "bitOrder") {
            m_bitOrder = UartBitOrder::from_string(value);
            return true;
        }
        if (key == "num_data_bits") {
            m_num_data_bits = NumDataBits::from_string(value);
            return true;
        }
        if (key == "parity") {
            m_parity = Parity::from_string(value);
            return true;
        }
        if (key == "num_stop_bits") {
            m_num_stop_bits = NumStopBits::from_string(value);
            return true;
        }
    } catch (...) {
        ERROR_LOG("Value %s not found in enumeration.", value.c_str())
    }
    return false;
}

auto UARTParameters::getDecoderSettingsString(std::string& key, std::string* value) -> bool {

    if (value == nullptr) {
        ERROR_LOG("Value is NULL")
        return false;
    }

    if (key == "rx") {
        *value = m_rx.name();
        return true;
    }
    if (key == "tx") {
        *value = m_tx.name();
        return true;
    }
    if (key == "invert") {
        *value = m_invert.name();
        return true;
    }
    if (key == "bitOrder") {
        *value = m_bitOrder.name();
        return true;
    }
    if (key == "num_data_bits") {
        *value = m_num_data_bits.name();
        return true;
    }
    if (key == "parity") {
        *value = m_parity.name();
        return true;
    }
    if (key == "num_stop_bits") {
        *value = m_num_stop_bits.name();
        return true;
    }
    return false;
}

auto UARTParameters::toJson() -> std::string {
    la_json::Builder root;

    root.add("rx", m_rx.name());
    root.add("tx", m_tx.name());
    root.add("baudrate", m_baudrate);
    root.add("invert", m_invert.name());
    root.add("bitOrder", m_bitOrder.name());
    root.add("num_data_bits", m_num_data_bits.name());
    root.add("parity", m_parity.name());
    root.add("num_stop_bits", m_num_stop_bits.name());
    root.add("acq_speed", m_samplerate);

    return root.str();
}

auto UARTParameters::fromJson(const std::string& json) -> bool {
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

        if (la_json::getString(root, "rx", text))
            m_rx = Lines::from_string(text);
        if (la_json::getString(root, "tx", text))
            m_tx = Lines::from_string(text);

        if (!parseUInt32(m_baudrate, "baudrate"))
            return false;

        if (la_json::getString(root, "invert", text))
            m_invert = InvertBit::from_string(text);

        if (la_json::getString(root, "bitOrder", text))
            m_bitOrder = UartBitOrder::from_string(text);

        if (la_json::getString(root, "num_data_bits", text))
            m_num_data_bits = NumDataBits::from_string(text);

        if (la_json::getString(root, "parity", text))
            m_parity = Parity::from_string(text);

        if (la_json::getString(root, "num_stop_bits", text))
            m_num_stop_bits = NumStopBits::from_string(text);

        if (!parseUInt32(m_samplerate, "acq_speed"))
            return false;

        return true;
    } catch (...) {
        ERROR_LOG("Error parse json. Invalid data")
        return false;
    }
    return false;
}

std::string UARTParameters::getUARTAnnotationsString(UARTAnnotations value) {
    switch (value) {
        case DATA:
            return "Data";
        case START_BIT:
            return "Start bit";
        case STOP_BIT:
            return "Stop ok";
        case PARITY_ERR:
            return "Parity bit error";
        case PARITY_BIT:
            return "Parity bit";
        case STOP_BIT_ERR:
            return "Stop bit error";
        case START_BIT_ERR:
            return "Start bit error";

        default:
            TRACE_SHORT("Unknown id = %d", (int)value)
            break;
    }
    return "";
}
