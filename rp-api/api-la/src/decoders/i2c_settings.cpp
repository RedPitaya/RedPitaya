#include <array>
#include <algorithm>
#include "i2c_settings.h"
#include "json/json.h"
#include "rp_log.h"

using namespace i2c;

constexpr const std::array<uint32_t,2> g_eAddressArray {Shifted, Unshifted};

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

I2CParameters::I2CParameters(){
	m_scl = 0;		// 0...8, 	0 if is not set
	m_sda = 0;		// 0...8, 	0 if is not set
	m_acq_speed = 0;
	m_address_format = Shifted;
	m_invert_bit = 0;
}

auto I2CParameters::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
	if (key == "scl"){
		m_scl = value;
		return true;
	}
	if (key == "sda"){
		m_sda = value;
		return true;
	}
	if (key == "acq_speed"){
		m_acq_speed = value;
		return true;
	}
	if (key == "address_format"){
		CHECK_ENUM(g_eAddressArray,value)
		m_address_format = (AddressFormat)value;
		return true;
	}
	if (key == "invert_bit"){
		m_invert_bit = value;
		return true;
	}
	return false;
}

auto I2CParameters::getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool {
	if (key == "scl"){
		*value = m_scl;
		return true;
	}
	if (key == "sda"){
		*value = m_sda;
		return true;
	}
	if (key == "acq_speed"){
		*value = m_acq_speed;
		return true;
	}
	if (key == "address_format"){
		*value = m_address_format;
		return true;
	}
	if (key == "invert_bit"){
		*value = m_invert_bit;
		return true;
	}
	return false;
}

auto I2CParameters::toJson() -> std::string{
	Json::Value root;

	root["scl"] = m_scl;
	root["sda"] = m_sda;
	root["acq_speed"] = m_acq_speed;
	root["address_format"] = m_address_format;
	root["invert_bit"] = m_invert_bit;

	Json::StreamWriterBuilder builder;
	const std::string json = Json::writeString(builder, root);
	return json;
}

auto I2CParameters::fromJson(const std::string &json) -> bool{
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

		if (!parseUInt32(m_scl,"scl")) return false;
		if (!parseUInt32(m_sda,"sda")) return false;
		if (!parseUInt32(m_acq_speed,"acq_speed")) return false;
		uint32_t x;
		if (!parseUInt32(x,"address_format")) return false;
		CHECK_ENUM(g_eAddressArray,x)
		m_address_format = (AddressFormat)x;
		if (!parseUInt32(m_invert_bit,"invert_bit")) return false;
		return true;
	} catch (...) {
		ERROR_LOG("Error parse json. Invalid data")
		return false;
	}
	return false;
}

std::string I2CParameters::getI2CAnnotationsString(I2CAnnotations value){
	switch (value)
	{
		case START: return "Start";
		case REPEAT_START: return "Repeat start";
		case STOP: return "Stop";

		case ACK: return "Ack";
		case NACK: return "Nack";

		case READ_ADDRESS: return "Read address";
		case WRITE_ADDRESS: return "Write address";

		case DATA_READ: return "Read data";
		case DATA_WRITE: return "Write data";
	default:
		ERROR_LOG("Unknown id = %d",(int)value)
		break;
	}
	return "";
}
