#include <array>
#include <algorithm>
#include "spi_settings.h"
#include "json/json.h"
#include "rp_log.h"

using namespace spi;

constexpr const std::array<uint32_t,2> g_eCsPolArray {ActiveLow,ActiveHigh};
constexpr const std::array<uint32_t,2> g_eBitOrderArray {MsbFirst, LsbFirst};

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

SPIParameters::SPIParameters(){
	m_clk = 0;		// 0...8, 	0 if is not set
	m_miso = 0; 	// 0...8,	0 if is not set
	m_mosi = 0; 	// 0...8,	0 if is not set
	m_cs = 0;  		// 0...8, 	0 if is not set
	m_cpol = 0;		// 0...1
	m_cpha = 0;		// 0...1
	m_word_size = 8;
	m_acq_speed = 0;
	m_cs_polarity = ActiveLow;
	m_bit_order = MsbFirst;
	m_invert_bit = 0;
}

auto SPIParameters::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
	if (key == "clk"){
		m_clk = value;
		return true;
	}
	if (key == "miso"){
		m_miso = value;
		return true;
	}
	if (key == "mosi"){
		m_mosi = value;
		return true;
	}
	if (key == "cs"){
		m_cs = value;
		return true;
	}
	if (key == "cpol"){
		m_cpol = value;
		return true;
	}
	if (key == "cpha"){
		m_cpha = value;
		return true;
	}
	if (key == "word_size"){
		m_word_size = value;
		return true;
	}
	if (key == "acq_speed"){
		m_acq_speed = value;
		return true;
	}
	if (key == "cs_polarity"){
		CHECK_ENUM(g_eCsPolArray,value)
		m_cs_polarity = (CsPolartiy)value;
		return true;
	}
	if (key == "bit_order"){
		CHECK_ENUM(g_eBitOrderArray,value)
		m_bit_order = (BitOrder)value;
		return true;
	}
	if (key == "invert_bit"){
		m_invert_bit = value;
		return true;
	}
	return false;
}

auto SPIParameters::getDecoderSettingsUInt(std::string& key, uint32_t *value) -> bool {
	if (key == "clk"){
		*value = m_clk;
		return true;
	}
	if (key == "miso"){
		*value = m_miso;
		return true;
	}
	if (key == "mosi"){
		*value = m_mosi;
		return true;
	}
	if (key == "cs"){
		*value = m_cs;
		return true;
	}
	if (key == "cpol"){
		*value = m_cpol;
		return true;
	}
	if (key == "cpha"){
		*value = m_cpha;
		return true;
	}
	if (key == "word_size"){
		*value = m_word_size;
		return true;
	}
	if (key == "acq_speed"){
		*value = m_acq_speed;
		return true;
	}
	if (key == "cs_polarity"){
		*value = m_cs_polarity;
		return true;
	}
	if (key == "bit_order"){
		*value = m_bit_order;
		return true;
	}
	if (key == "invert_bit"){
		*value = m_invert_bit;
		return true;
	}
	return false;
}

auto SPIParameters::toJson() -> std::string{
	Json::Value root;

	root["clk"] = m_clk;
	root["miso"] = m_miso;
	root["mosi"] = m_mosi;
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
		if (!parseUInt32(m_miso,"miso")) return false;
		if (!parseUInt32(m_mosi,"mosi")) return false;
		if (!parseUInt32(m_cs,"cs")) return false;

		if (!parseUInt32(m_cpol,"cpol")) return false;
		if (!parseUInt32(m_cpha,"cpha")) return false;
		if (!parseUInt32(m_word_size,"word_size")) return false;
		if (!parseUInt32(m_acq_speed,"acq_speed")) return false;

		uint32_t x;
		if (!parseUInt32(x,"cs_polarity")) return false;
		CHECK_ENUM(g_eCsPolArray,x)
		m_cs_polarity = (CsPolartiy)x;

		if (!parseUInt32(x,"bit_order")) return false;
		CHECK_ENUM(g_eBitOrderArray,x)
		m_bit_order = (BitOrder)x;

		if (!parseUInt32(m_invert_bit,"invert_bit")) return false;

		return true;
	} catch (...) {
		ERROR_LOG("Error parse json. Invalid data")
		return false;
	}
	return false;
}

std::string SPIParameters::getSPIAnnotationsString(SPIAnnotations value){
	switch (value)
	{
		case DATA: return "Data";
	default:
		TRACE_SHORT("Unknown id = %d",(int)value)
		break;
	}
	return "";
}
