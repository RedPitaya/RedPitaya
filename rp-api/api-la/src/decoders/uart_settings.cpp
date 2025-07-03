#include "uart_settings.h"
#include <algorithm>
#include <array>
#include "json/json.h"
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
    return false;
}

auto UARTParameters::getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool {
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

auto UARTParameters::toJson() -> std::string {
    Json::Value root;

    root["rx"] = m_rx.name();
    root["tx"] = m_tx.name();
    root["baudrate"] = m_baudrate;
    root["invert"] = m_invert.name();
    root["bitOrder"] = m_bitOrder.name();
    root["num_data_bits"] = m_num_data_bits.name();
    root["parity"] = m_parity.name();
    root["num_stop_bits"] = m_num_stop_bits.name();
    root["acq_speed"] = m_samplerate;

    Json::StreamWriterBuilder builder;
    const std::string json = Json::writeString(builder, root);
    return json;
}

auto UARTParameters::fromJson(const std::string& json) -> bool {
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

        if (root.isMember("rx"))
            m_rx = Lines::from_string(root["rx"].asString());
        if (root.isMember("tx"))
            m_tx = Lines::from_string(root["tx"].asString());

        if (!parseUInt32(m_baudrate, "baudrate"))
            return false;

        if (root.isMember("invert"))
            m_invert = InvertBit::from_string(root["invert"].asString());

        if (root.isMember("bitOrder"))
            m_bitOrder = UartBitOrder::from_string(root["bitOrder"].asString());

        if (root.isMember("num_data_bits"))
            m_num_data_bits = NumDataBits::from_string(root["num_data_bits"].asString());

        if (root.isMember("parity"))
            m_parity = Parity::from_string(root["parity"].asString());

        if (root.isMember("num_stop_bits"))
            m_num_stop_bits = NumStopBits::from_string(root["num_stop_bits"].asString());

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
