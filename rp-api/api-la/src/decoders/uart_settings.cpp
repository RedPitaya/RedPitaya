#include "uart_settings.h"
#include "json/json.h"
#include "rp_log.h"

using namespace uart;

UARTParameters::UARTParameters(){
	m_rx = 1; 	    // 1...8
	m_baudrate = 9600;
    m_invert_rx = 0;
    m_bitOrder = LSB_FIRST;
    m_num_data_bits = DATA_BITS_8;
    m_parity = NONE;
    m_num_stop_bits = STOP_BITS_NO;
    m_samplerate = 0;
}

auto UARTParameters::toJson() -> std::string{
	Json::Value root;

	root["rx"] = m_rx;
	root["baudrate"] = m_baudrate;
	root["invert_rx"] = m_invert_rx;
	root["bitOrder"] = m_bitOrder;
	root["num_data_bits"] = m_num_data_bits;
	root["parity"] = m_parity;
	root["num_stop_bits"] = m_num_stop_bits;
	root["samplerate"] = m_samplerate;

	Json::StreamWriterBuilder builder;
	const std::string json = Json::writeString(builder, root);
	return json;
}

auto UARTParameters::fromJson(const std::string &json) -> bool{
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

		auto parseUInt32 = [&](uint32_t &dest,std::string param){
			if (root.isMember(param)){
				dest = root[param].asUInt();
			}else{
				ERROR_LOG("Missing parameter %s",param.c_str())
				return false;
			}
			return true;
		};

		if (!parseUInt32(m_rx,"rx")) return false;
		if (!parseUInt32(m_baudrate,"baudrate")) return false;
		if (!parseUInt32(m_invert_rx,"invert_rx")) return false;

		uint32_t x;
		if (!parseUInt32(x,"bitOrder")) return false;
		m_bitOrder = (UartBitOrder)x;

		if (!parseUInt32(x,"num_data_bits")) return false;
		m_num_data_bits = (NumDataBits)x;

        if (!parseUInt32(x,"parity")) return false;
		m_parity = (Parity)x;

        if (!parseUInt32(x,"num_stop_bits")) return false;
		m_num_stop_bits = (NumStopBits)x;

		if (!parseUInt32(m_samplerate,"samplerate")) return false;

		return true;
	} catch (...) {
		ERROR_LOG("Error parse json. Invalid data")
		return false;
	}
	return false;
}

std::string UARTParameters::getUARTAnnotationsString(UARTAnnotations value){
	switch (value)
	{
		case NO: return "";
		case RX_DATA: return "Data";
		case RX_START: return "Start";
		case RX_PARITY_OK: return "Parity ok";
		case RX_PARITY_ERR: return "Parity err";
		case RX_STOP: return "Stop";
		case RX_WARNING: return "Warning";
		case RX_DATA_BITS: return "Data bits";

	default:
		TRACE_SHORT("Unknown id = %d",(int)value)
		break;
	}
	return "";
}
