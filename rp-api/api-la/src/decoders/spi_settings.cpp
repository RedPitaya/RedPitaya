#include "spi_settings.h"
#include "json/json.h"
#include "rp_log.h"

using namespace spi;

SPIParameters::SPIParameters(){
	m_clk = 1; 	// 1...8 	ch Number
	m_data = 1; 	// 1...8
	m_cs = 0;  	// 0...8, 	0 if is not set
	m_cpol = 0;
	m_cpha = 0;
	m_word_size = 8;
	m_acq_speed = 0;
	m_cs_polarity = ActiveLow;
	m_bit_order = MsbFirst;
	m_invert_bit = 0;
}

auto SPIParameters::toJson() -> std::string{
	Json::Value root;

	root["clk"] = m_clk;
	root["data"] = m_data;
	root["cs"] = m_cs;
	root["cpol"] = m_cpol;
	root["cpha"] = m_cpha;
	root["word_size"] = m_word_size;
	root["acq_speed"] = m_acq_speed;
	root["cs_polarity"] = m_cs_polarity;
	root["bit_order"] = m_bit_order;
	root["invert_bit"] = m_invert_bit;

	Json::StreamWriterBuilder builder;
	const std::string json = Json::writeString(builder, root);
	return json;
}

auto SPIParameters::fromJson(const std::string &json) -> bool{
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

		if (!parseUInt32(m_clk,"clk")) return false;
		if (!parseUInt32(m_data,"data")) return false;
		if (!parseUInt32(m_cs,"cs")) return false;

		if (!parseUInt32(m_cpol,"cpol")) return false;
		if (!parseUInt32(m_cpha,"cpha")) return false;
		if (!parseUInt32(m_word_size,"word_size")) return false;
		if (!parseUInt32(m_acq_speed,"acq_speed")) return false;

		uint32_t x;
		if (!parseUInt32(x,"cs_polarity")) return false;
		m_cs_polarity = (CsPolartiy)x;

		if (!parseUInt32(x,"bit_order")) return false;
		m_bit_order = (BitOrder)x;

		if (!parseUInt32(m_invert_bit,"invert_bit")) return false;

		return true;
	} catch (...) {
		ERROR_LOG("Error parse json. Invalid file")
		return false;
	}
	return false;
}

 constexpr std::string_view SPIParameters::getSPIAnnotationsString(SPIAnnotations value){
	switch (value)
	{
		case DATA: return "Data";
		case NOTHING: return "";

	default:
		ERROR_LOG("Unknown id = %d",(int)value)
		break;
	}
	return "";
}
