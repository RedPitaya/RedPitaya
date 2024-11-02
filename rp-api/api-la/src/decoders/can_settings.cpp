#include "can_settings.h"
#include "json/json.h"
#include "rp_log.h"

using namespace can;

CANParameters::CANParameters(){
	m_can_rx = 1;
	m_nominal_bitrate = 1000000;
	m_fast_bitrate = 2000000;
	m_sample_point = 87.5;
	m_invert_bit = 0;
	m_frame_limit = 500;
}

auto CANParameters::toJson() -> std::string{
	Json::Value root;

	root["can_rx"] = m_can_rx;
	root["nominal_bitrate"] = m_nominal_bitrate;
	root["fast_bitrate"] = m_fast_bitrate;
	root["sample_point"] = m_sample_point;
	root["invert_bit"] = m_invert_bit;
	root["frame_limit"] = m_frame_limit;

	Json::StreamWriterBuilder builder;
	const std::string json = Json::writeString(builder, root);
	return json;
}

auto CANParameters::fromJson(const std::string &json) -> bool{
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

		auto parseFloat = [&](float &dest,std::string param){
			if (root.isMember(param)){
				dest = root[param].asFloat();
			}else{
				ERROR_LOG("Missing parameter %s",param.c_str())
				return false;
			}
			return true;
		};

		if (!parseUInt32(m_can_rx,"can_rx")) return false;
		if (!parseUInt32(m_nominal_bitrate,"nominal_bitrate")) return false;
		if (!parseUInt32(m_fast_bitrate,"fast_bitrate")) return false;
		if (!parseFloat(m_sample_point,"sample_point")) return false;
		if (!parseUInt32(m_frame_limit,"frame_limit")) return false;
		if (!parseUInt32(m_acq_speed,"acq_speed")) return false;
		if (!parseUInt32(m_invert_bit,"invert_bit")) return false;

		return true;
	} catch (...) {
		ERROR_LOG("Error parse json. Invalid file")
		return false;
	}
	return false;
}

 constexpr std::string_view CANParameters::getCANAnnotationsString(CANAnnotations value){
	switch (value)
	{
		case PAYLOAD_DATA: return "Payload data";
		case START_OF_FRAME: return "Start of frame";
		case END_OF_FRAME: return "End of frame";

		case ID: return "Identifier";
		case EXT_ID: return "Extended identifier";
		case FULL_ID: return "Full identifier";

		case IDE: return "Identifier extension bit";
		case RESERV_BIT: return "Reserved bit 0 and 1";

		case RTR: return "Remote transmission request";
		case SRR: return "Substitute remote request";
		case DLC: return "Data length count";

		case CRC_SEQ: return "CRC sequence";
		case CRC_DELIMITER: return "CRC delimiter";
		case ACK_SLOT: return "ACK slot";
		case ACK_DELIMITER: return "ACK delimiter";

		case STUFF_BIT: return "Stuff bit";

		case WARNING: return "Warning unknow";
		case BIT: return "Bit";
		case ERROR_1: return "Start of frame (SOF) must be a dominant bit";
		case ERROR_2: return "Data length code (DLC) > 8 is not allowed";
		case ERROR_3: return "End of frame (EOF) must be 7 recessive bits";

		case WARNING_1: return "Identifier bits 10..4 must not be all recessive";
		case WARNING_2: return "CRC delimiter must be a recessive bit";
		case WARNING_3: return "ACK delimiter must be a recessive bit";

		case BRS: return "Bit rate switch";
		case ESI: return "Error state indicator";
		case CRC_LEN: return "Crc type";
		case RESERV_BIT_FLEX: return "Flexible data";
		case NOTHING: return "";
		case SYNC: return "Sync";

	default:
		ERROR_LOG("Unknown id = %d",(int)value)
		break;
	}
	return "";
}
