#include "i2c_settings.h"
#include "json/json.h"
#include "rp_log.h"

using namespace i2c;

I2CParameters::I2CParameters(){
	m_scl = 1;
	m_sda = 1;
	m_acq_speed = 0;
	m_address_format = Shifted;
	m_invert_bit = 0;
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
		case NOTHING: return "";

	default:
		ERROR_LOG("Unknown id = %d",(int)value)
		break;
	}
	return "";
}
