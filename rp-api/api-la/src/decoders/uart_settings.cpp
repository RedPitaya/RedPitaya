#include <array>
#include <algorithm>
#include "uart_settings.h"
#include "json/json.h"
#include "rp_log.h"

using namespace uart;

constexpr const std::array<uint32_t,2> g_eBitOrderArray {LSB_FIRST,MSB_FIRST};
constexpr const std::array<uint32_t,5> g_eNumBitArray {DATA_BITS_5, DATA_BITS_6, DATA_BITS_7, DATA_BITS_8, DATA_BITS_9};
constexpr const std::array<uint32_t,5> g_eParityArray {NONE, EVEN, ODD, ALWAYS_0, ALWAYS_1};
constexpr const std::array<uint32_t,5> g_eStopBitsArray {STOP_BITS_NO, STOP_BITS_05, STOP_BITS_10, STOP_BITS_15, STOP_BITS_20};

#define IS(X,Y) (std::find(std::begin(X), std::end(X), Y) != std::end(X))
#define CHECK_ENUM(X,Y) \
	if (!IS(X,Y)){ \
			ERROR_LOG("Invalid value: %d. Valid parameters: %s",Y,getValues(X).c_str()) \
			return false; \
	}

template<typename X>
auto getValues(X array) -> std::string{
	std::string values = "";
	bool skip = true;
	for(size_t i = 0; i < array.size(); i++){
		if (!skip){
			values += ",";
		}
		values += std::to_string(array[i]);
		skip = false;
	}
	return values;
}

UARTParameters::UARTParameters(){
	m_rx = 0; 	    // 1...8, 0 if is not set
	m_tx = 0; 	    // 1...8, 0 if is not set
	m_baudrate = 9600;
    m_invert = 0;
    m_bitOrder = LSB_FIRST;
    m_num_data_bits = DATA_BITS_8;
    m_parity = NONE;
    m_num_stop_bits = STOP_BITS_10;
    m_samplerate = 0;
}

auto UARTParameters::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
	if (key == "rx"){
		m_rx = value;
		return true;
	}
	if (key == "tx"){
		m_tx = value;
		return true;
	}
	if (key == "baudrate"){
		m_baudrate = value;
		return true;
	}
	if (key == "invert"){
		m_invert = value;
		return true;
	}
	if (key == "bitOrder"){
		CHECK_ENUM(g_eBitOrderArray,value)
		m_bitOrder = (UartBitOrder)value;
		return true;
	}
	if (key == "num_data_bits"){
		CHECK_ENUM(g_eNumBitArray,value)
		m_num_data_bits = (NumDataBits)value;
		return true;
	}
	if (key == "parity"){
		CHECK_ENUM(g_eParityArray,value)
		m_parity = (Parity)value;
		return true;
	}
	if (key == "num_stop_bits"){
		CHECK_ENUM(g_eStopBitsArray,value)
		m_num_stop_bits = (NumStopBits)value;
		return true;
	}
	if (key == "acq_speed"){
		m_samplerate = value;
		return true;
	}
	return false;
}

auto UARTParameters::getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool {
	if (key == "rx"){
		*value = m_rx;
		return true;
	}
	if (key == "tx"){
		*value = m_tx;
		return true;
	}
	if (key == "baudrate"){
		*value = m_baudrate;
		return true;
	}
	if (key == "invert"){
		*value = m_invert;
		return true;
	}
	if (key == "bitOrder"){
		*value = m_bitOrder;
		return true;
	}
	if (key == "num_data_bits"){
		*value = m_num_data_bits;
		return true;
	}
	if (key == "parity"){
		*value = m_parity;
		return true;
	}
	if (key == "num_stop_bits"){
		*value = m_num_stop_bits;
		return true;
	}
	if (key == "acq_speed"){
		*value = m_samplerate;
		return true;
	}
	return false;
}

auto UARTParameters::toJson() -> std::string{
	Json::Value root;

	root["rx"] = m_rx;
	root["tx"] = m_tx;
	root["baudrate"] = m_baudrate;
	root["invert"] = m_invert;
	root["bitOrder"] = m_bitOrder;
	root["num_data_bits"] = m_num_data_bits;
	root["parity"] = m_parity;
	root["num_stop_bits"] = m_num_stop_bits;
	root["acq_speed"] = m_samplerate;

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
		if (!parseUInt32(m_tx,"tx")) return false;
		if (!parseUInt32(m_baudrate,"baudrate")) return false;
		if (!parseUInt32(m_invert,"invert")) return false;

		uint32_t x;
		if (!parseUInt32(x,"bitOrder")) return false;
		CHECK_ENUM(g_eBitOrderArray,x)
		m_bitOrder = (UartBitOrder)x;

		if (!parseUInt32(x,"num_data_bits")) return false;
		CHECK_ENUM(g_eNumBitArray,x)
		m_num_data_bits = (NumDataBits)x;

        if (!parseUInt32(x,"parity")) return false;
		CHECK_ENUM(g_eParityArray,x)
		m_parity = (Parity)x;

        if (!parseUInt32(x,"num_stop_bits")) return false;
		CHECK_ENUM(g_eStopBitsArray,x)
		m_num_stop_bits = (NumStopBits)x;

		if (!parseUInt32(m_samplerate,"acq_speed")) return false;

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
		case DATA: return "Data";
		case START_BIT: return "Start bit";
		case STOP_BIT: return "Stop ok";
		case PARITY_ERR: return "Parity bit error";
		case PARITY_BIT: return "Parity bit";
		case STOP_BIT_ERR: return "Stop bit error";
		case START_BIT_ERR: return "Start bit error";

	default:
		TRACE_SHORT("Unknown id = %d",(int)value)
		break;
	}
	return "";
}
