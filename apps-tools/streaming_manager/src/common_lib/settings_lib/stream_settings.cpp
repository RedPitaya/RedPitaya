#include "stream_settings.h"
#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "json/json.h"
#include <filesystem>
#include <fstream>
#include <iostream>

#define FILE_PATH "/home/redpitaya/streaming_files/dac"

using namespace std;

std::string g_dirPath = {FILE_PATH};

int to_int(char const *s)
{
	if (s == NULL || *s == '\0')
		throw std::invalid_argument("null or empty string argument");

	bool negate = (s[0] == '-');
	if (*s == '+' || *s == '-')
		++s;

	if (*s == '\0')
		throw std::invalid_argument("sign character only.");

	int result = 0;
	while (*s) {
		if (*s < '0' || *s > '9')
			throw std::invalid_argument("invalid input string");
		result = result * 10 - (*s - '0'); //assume negative number
		++s;
	}
	return negate ? result : -result; //-result is positive!
}

uint32_t to_uint(char const *s)
{
	if (s == NULL || *s == '\0')
		throw std::invalid_argument("null or empty string argument");

	if (*s == '\0')
		throw std::invalid_argument("sign character only.");

	uint32_t result = 0;
	while (*s) {
		if (*s < '0' || *s > '9')
			throw std::invalid_argument("invalid input string");
		int x = (*s - '0');
		result = result * 10 + x;
		++s;
	}
	return result; //-result is positive!
}

uint64_t to_uint64(char const *s)
{
	if (s == NULL || *s == '\0')
		throw std::invalid_argument("null or empty string argument");

	if (*s == '\0')
		throw std::invalid_argument("sign character only.");

	uint64_t result = 0;
	while (*s) {
		if (*s < '0' || *s > '9')
			throw std::invalid_argument("invalid input string");
		int x = (*s - '0');
		result = result * 10 + x;
		++s;
	}
	return result; //-result is positive!
}

CStreamSettings::CStreamSettings()
{
	resetDefault();
}

auto CStreamSettings::resetDefault() -> void
{
	m_adcsettings = ADCSettings();
	m_memorysettings = MemorySettings();
	m_dacsettings = DACSettings();
}

CStreamSettings::~CStreamSettings() {}

CStreamSettings::CStreamSettings(const CStreamSettings &src)
{
	copy(src);
}

CStreamSettings &CStreamSettings::operator=(const CStreamSettings &src)
{
	copy(src);
	return *this;
}

auto CStreamSettings::copy(const CStreamSettings &src) -> void
{
	m_adcsettings = src.m_adcsettings;
	m_dacsettings = src.m_dacsettings;
	m_memorysettings = src.m_memorysettings;
}

bool CStreamSettings::writeToFile(string _filename)
{
	const std::string json_file = toJson();

	try {
		auto path = filesystem::path(_filename);
		filesystem::create_directories(path.parent_path());
	} catch (std::filesystem::filesystem_error const &ex) {
		ERROR_LOG("Error create dir %s", ex.what())
	}

	ofstream file(_filename, ios::out | ios::trunc);
	if (!file.is_open()) {
		ERROR_LOG("File write failed %s", std::strerror(errno))
		return false;
	}

	file << json_file;
	file.close();
	return true;
}

auto CStreamSettings::toJson() const -> std::string
{
	Json::Value root;
	Json::Value adc_config;
	Json::Value dac_config;
	Json::Value memory_config;

	adc_config["format_sd"] = getADCFormat().name();
	adc_config["data_type_sd"] = getADCType().name();
	adc_config["samples_limit_sd"] = getADCSamples();

	adc_config["adc_pass_mode"] = getADCPassMode().name();
	adc_config["resolution"] = getADCResolution().name();
	adc_config["adc_decimation"] = getADCDecimation();
	adc_config["use_calib"] = getADCCalibration().name();

	for (auto i = 1u; i <= 4; i++) {
		adc_config["channel_state_" + to_string(i)] = getADCChannels(i).name();
		adc_config["channel_attenuator_" + to_string(i)] = getADCAttenuator(i).name();
		adc_config["channel_ac_dc_" + to_string(i)] = getADCAC_DC(i).name();
	}

	dac_config["dac_rate"] = getDACSpeed();
	dac_config["file_sd"] = getDACFile();
	dac_config["file_type_sd"] = getDACFileType().name();
	dac_config["dac_pass_mode"] = getDACPassMode().name();

	dac_config["repeat"] = getDACRepeat().name();
	dac_config["repeatCount"] = getDACRepeatCount();

	for (auto i = 1u; i <= 2; i++) {
		dac_config["channel_gain_" + to_string(i)] = getDACGain(i).name();
	}

	memory_config["block_size"] = getMemoryBlockSize();
	memory_config["adc_size"] = getADCSize();
	memory_config["dac_size"] = getDACSize();
	memory_config["gpio_size"] = getGPIOSize();

	root["adc_streaming"] = adc_config;
	root["dac_streaming"] = dac_config;
	root["memory_manager"] = memory_config;

	Json::StreamWriterBuilder builder;
	const std::string json = Json::writeString(builder, root);
	return json;
}

auto CStreamSettings::parseJson(const std::string &json) -> bool
{
	Json::Value root;
	Json::Value adc_config;
	Json::Value dac_config;
	Json::Value memory_config;

	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	JSONCPP_STRING errs;
	auto is = std::istringstream(json);
	if (!parseFromStream(builder, is, &root, &errs)) {
		std::cerr << "[CStreamSettings] Error parse json" << errs << std::endl;
		return false;
	}

	if (root.isMember("adc_streaming")) {
		adc_config = root["adc_streaming"];
	} else {
		std::cerr << "[CStreamSettings] Error parse json. Invalid file" << std::endl;
		return false;
	}

	if (root.isMember("dac_streaming")) {
		dac_config = root["dac_streaming"];
	} else {
		std::cerr << "[CStreamSettings] Error parse json. Invalid file" << std::endl;
		return false;
	}

	if (root.isMember("memory_manager")) {
		memory_config = root["memory_manager"];
	} else {
		std::cerr << "[CStreamSettings] Error parse json. Invalid file" << std::endl;
		return false;
	}

	try {
		if (adc_config.isMember("format_sd"))
			setADCFormat(DataFormat::from_string(adc_config["format_sd"].asString()));
		if (adc_config.isMember("data_type_sd"))
			setADCType(DataType::from_string(adc_config["data_type_sd"].asString()));
		if (adc_config.isMember("samples_limit_sd"))
			setADCSamples(adc_config["samples_limit_sd"].asUInt64());
		if (adc_config.isMember("adc_pass_mode"))
			setADCPassMode(PassMode::from_string(adc_config["adc_pass_mode"].asString()));
		if (adc_config.isMember("resolution"))
			setADCResolution(Resolution::from_string(adc_config["resolution"].asString()));
		if (adc_config.isMember("adc_decimation"))
			setADCDecimation(adc_config["adc_decimation"].asUInt());
		if (adc_config.isMember("use_calib"))
			setADCCalibration(State::from_string(adc_config["use_calib"].asString()));
		for (auto i = 1u; i <= 4; i++) {
			if (adc_config.isMember("channel_state_" + to_string(i)))
				setADCChannels(i, State::from_string(adc_config["channel_state_" + to_string(i)].asString()));
			if (adc_config.isMember("channel_attenuator_" + to_string(i)))
				setADCAttenuator(i, Attenuator::from_string(adc_config["channel_attenuator_" + to_string(i)].asString()));
			if (adc_config.isMember("channel_attenuator_" + to_string(i)))
				setADCAC_DC(i, AC_DC::from_string(adc_config["channel_ac_dc_" + to_string(i)].asString()));
		}

		if (dac_config.isMember("dac_rate"))
			setDACSpeed(dac_config["dac_rate"].asUInt());
		if (dac_config.isMember("file_sd"))
			setDACFile(dac_config["file_sd"].asString());
		if (dac_config.isMember("file_type_sd"))
			setDACFileType(DataFormat::from_string(dac_config["file_type_sd"].asString()));
		if (dac_config.isMember("dac_pass_mode"))
			setDACPassMode(DACPassMode::from_string(dac_config["dac_pass_mode"].asString()));
		if (dac_config.isMember("repeat"))
			setDACRepeat(DACRepeat::from_string(dac_config["repeat"].asString()));
		if (dac_config.isMember("repeatCount"))
			setDACRepeatCount(dac_config["repeatCount"].asUInt());
		for (auto i = 1u; i <= 2; i++) {
			if (dac_config.isMember("channel_gain_" + to_string(i)))
				setDACGain(i, DACGain::from_string(dac_config["channel_gain_" + to_string(i)].asString()));
		}

		if (memory_config.isMember("block_size"))
			setMemoryBlockSize(memory_config["block_size"].asUInt());
		if (memory_config.isMember("adc_size"))
			setADCSize(memory_config["adc_size"].asUInt());
		if (memory_config.isMember("dac_size"))
			setDACSize(memory_config["dac_size"].asUInt());
		if (memory_config.isMember("gpio_size"))
			setGPIOSize(memory_config["gpio_size"].asUInt());
		return true;
	} catch (...) {
		ERROR_LOG("Error parse json. Invalid file")
		return false;
	}
	return false;
}

auto CStreamSettings::toString() const -> std::string
{
	std::string str = "******************** ADC streaming ********************\n";

	std::string channels = "";
	for (auto i = 1; i <= 4; i++) {
		channels += "\tCh " + std::to_string(i) + ": " + getADCChannels(i).to_string() + " Attenuator: " + getADCAttenuator(i).to_string()
					+ " AD/DC (250-12 only): " + getADCAC_DC(i).to_string() + "\n";
	}

	str += "Channels:\n" + channels;
	str += "Decimation:\t\t" + std::to_string(getADCDecimation()) + "\n";
	str += "Resolution:\t\t" + std::string(getADCResolution().to_string()) + "\n";
	str += "Use calibration:\t\t" + std::string(getADCCalibration().to_string()) + "\n";
	str += "Pass mode:\t\t" + std::string(getADCPassMode().to_string()) + "\n";
	str += "Samples:\t\t" + (getADCSamples() == 0 ? "Unlimited" : std::to_string(getADCSamples())) + " (In file mode)\n";
	str += "Data format:\t\t" + std::string(getADCFormat().to_string()) + " (In file mode)\n";
	str += "Data type:\t\t" + std::string(getADCType().to_string()) + " (In file mode)\n";

	str += "\n******************** DAC streaming ********************\n";
	channels = "";
	for (auto i = 1; i <= 4; i++) {
		channels += "\tCh " + std::to_string(i) + " Gain (250-12 only): " + getDACGain(i).to_string() + "\n";
	}
	str += "Channels:\n" + channels;
	str += "Rate:\t\t" + std::to_string(getDACSpeed()) + "\n";
	str += "Pass mode:\t\t" + std::string(getDACPassMode().to_string()) + "\n";
	str += "Path to local file on SD:\t\t" + getDACFile() + "\n";
	str += "Data format:\t\t" + std::string(getDACFileType().to_string()) + " (In file mode)\n";
	str += "Repeat:\t\t" + std::string(getDACRepeat().to_string()) + " (In file mode)\n";
	str += "Repeat count:\t" + std::to_string(getDACRepeatCount()) + " (In file mode)\n";

	str += "\n******************** Memory Manager *******************\n";
	str += "Block size:\t\t" + std::to_string(getMemoryBlockSize()) + "\n";
	str += "ADC size:\t\t" + std::to_string(getADCSize()) + "\n";
	str += "DAC size:\t\t" + std::to_string(getDACSize()) + "\n";
	str += "GPIO size:\t\t" + std::to_string(getGPIOSize()) + "\n";

	return str;
}

auto CStreamSettings::readFromFile(string _filename) -> bool
{
	std::ifstream file(_filename, ios::in);
	if (!file.is_open()) {
		ERROR_LOG("File %s read failed: %s", _filename.c_str(), std::strerror(errno))
		return false;
	}
	resetDefault();
	std::stringstream strStream;
	strStream << file.rdbuf();
	return parseJson(strStream.str());
}

auto CStreamSettings::setADCSamples(uint64_t _samples) -> void
{
	m_adcsettings.m_samples = _samples;
}

auto CStreamSettings::getADCSamples() const -> uint64_t
{
	return m_adcsettings.m_samples;
}

auto CStreamSettings::setADCFormat(CStreamSettings::DataFormat _format) -> void
{
	m_adcsettings.m_format = _format;
}

auto CStreamSettings::getADCFormat() const -> CStreamSettings::DataFormat
{
	return m_adcsettings.m_format;
}

auto CStreamSettings::setADCType(DataType _type) -> void
{
	m_adcsettings.m_dataType = _type;
}

auto CStreamSettings::getADCType() const -> CStreamSettings::DataType
{
	return m_adcsettings.m_dataType;
}

auto CStreamSettings::setADCPassMode(CStreamSettings::PassMode _type) -> void
{
	m_adcsettings.m_passMode = _type;
}

auto CStreamSettings::getADCPassMode() const -> CStreamSettings::PassMode
{
	return m_adcsettings.m_passMode;
}

auto CStreamSettings::setADCChannels(uint8_t _channel, CStreamSettings::State _state) -> bool
{
	if (_channel > 0 && _channel <= 4) {
		m_adcsettings.m_channels[_channel - 1] = _state;
		return true;
	}
	return false;
}

auto CStreamSettings::getADCChannels(uint8_t _channel) const -> CStreamSettings::State
{
	if (_channel > 0 && _channel <= 4) {
		return m_adcsettings.m_channels[_channel - 1];
	}
	return CStreamSettings::State::OFF;
}

auto CStreamSettings::setADCResolution(CStreamSettings::Resolution _value) -> void
{
	m_adcsettings.m_resolution = _value;
}

auto CStreamSettings::getADCResolution() const -> CStreamSettings::Resolution
{
	return m_adcsettings.m_resolution;
}

auto CStreamSettings::setADCDecimation(uint32_t _decimation) -> bool
{
	if (_decimation > 1024 * 64)
		return false;
	m_adcsettings.m_decimation = _decimation;
	return true;
}

auto CStreamSettings::getADCDecimation() const -> uint32_t
{
	return m_adcsettings.m_decimation;
}

auto CStreamSettings::setADCAttenuator(uint8_t _channel, Attenuator _state) -> bool
{
	if (_channel > 0 && _channel <= 4) {
		m_adcsettings.m_attenuator[_channel - 1] = _state;
		return true;
	}
	return false;
}

auto CStreamSettings::getADCAttenuator(uint8_t _channel) const -> Attenuator
{
	if (_channel > 0 && _channel <= 4) {
		return m_adcsettings.m_attenuator[_channel - 1];
	}
	return CStreamSettings::Attenuator::A_1_1;
}

auto CStreamSettings::setADCAC_DC(uint8_t _channel, AC_DC _state) -> bool
{
	if (_channel > 0 && _channel <= 4) {
		m_adcsettings.m_ac_dc[_channel - 1] = _state;
		return true;
	}
	return false;
}

auto CStreamSettings::getADCAC_DC(uint8_t _channel) const -> AC_DC
{
	if (_channel > 0 && _channel <= 4) {
		return m_adcsettings.m_ac_dc[_channel - 1];
	}
	return CStreamSettings::AC_DC::DC;
}

auto CStreamSettings::setADCCalibration(State _calibration) -> void
{
	m_adcsettings.m_useCalib = _calibration;
}

auto CStreamSettings::getADCCalibration() const -> State
{
	return m_adcsettings.m_useCalib;
}

auto CStreamSettings::setDACFile(std::string _value) -> void
{
	m_dacsettings.m_file = _value;
}

auto CStreamSettings::getDACFile() const -> std::string
{
	return m_dacsettings.m_file;
}

auto CStreamSettings::setDACFileType(CStreamSettings::DataFormat _value) -> bool
{
	if (_value.value != DataFormat::BIN) {
		m_dacsettings.m_file_type = _value;
		return true;
	}
	return false;
}

auto CStreamSettings::getDACFileType() const -> CStreamSettings::DataFormat
{
	return m_dacsettings.m_file_type;
}

auto CStreamSettings::setDACGain(uint8_t _channel, DACGain _value) -> bool
{
	if (_channel > 0 && _channel <= 2) {
		m_dacsettings.m_gain[_channel - 1] = _value;
		return true;
	}
	return false;
}

auto CStreamSettings::getDACGain(uint8_t _channel) const -> DACGain
{
	if (_channel > 0 && _channel <= 2) {
		return m_dacsettings.m_gain[_channel - 1];
	}
	return CStreamSettings::DACGain::X1;
}

auto CStreamSettings::setDACPassMode(CStreamSettings::DACPassMode _value) -> void
{
	m_dacsettings.m_passMode = _value;
}

auto CStreamSettings::getDACPassMode() const -> CStreamSettings::DACPassMode
{
	return m_dacsettings.m_passMode;
}

auto CStreamSettings::setDACRepeat(DACRepeat _value) -> void
{
	m_dacsettings.m_repeat = _value;
}

auto CStreamSettings::getDACRepeat() const -> DACRepeat
{
	return m_dacsettings.m_repeat;
}

auto CStreamSettings::setDACSpeed(uint32_t _value) -> bool
{
	m_dacsettings.m_rate = _value;
	return true;
}

auto CStreamSettings::getDACSpeed() const -> uint32_t
{
	return m_dacsettings.m_rate;
}

auto CStreamSettings::setDACRepeatCount(uint32_t _value) -> void
{
	m_dacsettings.m_repeatCount = _value;
}

auto CStreamSettings::getDACRepeatCount() const -> uint32_t
{
	return m_dacsettings.m_repeatCount;
}

auto CStreamSettings::setMemoryBlockSize(uint32_t size) -> void
{
	m_memorysettings.m_memoryBlock = size;
}

auto CStreamSettings::getMemoryBlockSize() const -> uint32_t
{
	return m_memorysettings.m_memoryBlock;
}

auto CStreamSettings::setADCSize(uint32_t count) -> void
{
	m_memorysettings.m_ADCSize = count;
}

auto CStreamSettings::getADCSize() const -> uint32_t
{
	return m_memorysettings.m_ADCSize;
}

auto CStreamSettings::setDACSize(uint32_t count) -> void
{
	m_memorysettings.m_DACSize = count;
}

auto CStreamSettings::getDACSize() const -> uint32_t
{
	return m_memorysettings.m_DACSize;
}

auto CStreamSettings::setGPIOSize(uint32_t count) -> void
{
	m_memorysettings.m_GPIOSize = count;
}

auto CStreamSettings::getGPIOSize() const -> uint32_t
{
	return m_memorysettings.m_GPIOSize;
}

auto CStreamSettings::setValue(std::string key, std::string value) -> bool
{
	try {
		if (key == "format_sd") {
			setADCFormat(DataFormat::from_string(value));
			return true;
		}

		if (key == "data_type_sd") {
			setADCType(DataType::from_string(value));
			return true;
		}

		if (key == "samples_limit_sd") {
			setADCSamples(to_uint64(value.c_str()));
			return true;
		}

		if (key == "adc_pass_mode") {
			setADCPassMode(PassMode::from_string(value));
			return true;
		}

		if (key == "resolution") {
			setADCResolution(Resolution::from_string(value));
			return true;
		}

		if (key == "adc_decimation") {
			return setADCDecimation(to_uint(value.c_str()));
		}

		if (key == "use_calib") {
			setADCCalibration(State::from_string(value));
			return true;
		}

		for (auto i = 1u; i <= 4; i++) {
			if (key == "channel_state_" + to_string(i)) {
				return setADCChannels(i, State::from_string(value));
			}

			if (key == "channel_attenuator_" + to_string(i)) {
				return setADCAttenuator(i, Attenuator::from_string(value));
			}

			if (key == "channel_ac_dc_" + to_string(i)) {
				return setADCAC_DC(i, AC_DC::from_string(value));
			}
		}

		if (key == "dac_rate") {
			return setDACSpeed(to_uint(value.c_str()));
		}

		if (key == "file_sd") {
			setDACFile((value));
			return true;
		}

		if (key == "file_type_sd") {
			setDACFileType(DataFormat::from_string(value));
			return true;
		}

		if (key == "dac_pass_mode") {
			setDACPassMode(DACPassMode::from_string(value));
			return true;
		}

		if (key == "repeat") {
			setDACRepeat(DACRepeat::from_string(value));
			return true;
		}

		if (key == "repeatCount") {
			setDACRepeatCount(to_uint(value.c_str()));
			return true;
		}

		for (auto i = 1u; i <= 2; i++) {
			if (key == "channel_gain_" + to_string(i)) {
				return setDACGain(i, DACGain::from_string(value));
			}
		}

		if (key == "block_size") {
			setMemoryBlockSize(to_uint(value.c_str()));
			return true;
		}

		if (key == "adc_size") {
			setADCSize(to_uint(value.c_str()));
			return true;
		}

		if (key == "dac_size") {
			setDACSize(to_uint(value.c_str()));
			return true;
		}

		if (key == "gpio_size") {
			setGPIOSize(to_uint(value.c_str()));
			return true;
		}

	} catch (...) {
		return false;
	}
	return false;
}

auto CStreamSettings::getValue(std::string key) -> std::string
{
	try {
		if (key == "format_sd") {
			return getADCFormat().name();
		}

		if (key == "data_type_sd") {
			return getADCType().name();
		}

		if (key == "samples_limit_sd") {
			return std::to_string(getADCSamples());
		}

		if (key == "adc_pass_mode") {
			return getADCPassMode().name();
		}

		if (key == "resolution") {
			return getADCResolution().name();
		}

		if (key == "adc_decimation") {
			return std::to_string(getADCDecimation());
		}

		if (key == "use_calib") {
			return getADCCalibration().name();
		}

		for (auto i = 1u; i <= 4; i++) {
			if (key == "channel_state_" + to_string(i)) {
				return getADCChannels(i).name();
			}

			if (key == "channel_attenuator_" + to_string(i)) {
				return getADCAttenuator(i).name();
			}

			if (key == "channel_ac_dc_" + to_string(i)) {
				return getADCAC_DC(i).name();
			}
		}

		if (key == "dac_rate") {
			return std::to_string(getDACSpeed());
		}

		if (key == "file_sd") {
			return getDACFile();
		}

		if (key == "file_type_sd") {
			return getDACFileType().name();
		}

		if (key == "dac_pass_mode") {
			return getDACPassMode().name();
		}

		if (key == "repeat") {
			return getDACRepeat().name();
		}

		if (key == "repeatCount") {
			return std::to_string(getDACRepeatCount());
		}

		for (auto i = 1u; i <= 2; i++) {
			if (key == "channel_gain_" + to_string(i)) {
				return getDACGain(i).name();
			}
		}

		if (key == "block_size") {
			return std::to_string(getMemoryBlockSize());
		}

		if (key == "adc_size") {
			return std::to_string(getADCSize());
			;
		}

		if (key == "dac_size") {
			return std::to_string(getDACSize());
			;
		}

		if (key == "gpio_size") {
			return std::to_string(getGPIOSize());
			;
		}

	} catch (...) {
		return "ERROR";
	}
	return "ERROR";
}

auto CStreamSettings::getHelp() -> std::string
{
	auto concat = [](const char *const *name, int count) -> std::string {
		std::string s = "";
		bool first = true;
		for (int i = 0; i < count; i++) {
			if (!first)
				s += ",";
			s += std::string(name[i]);
			first = false;
		}
		return s;
	};

	std::string s = "";
	s += "format_sd\t\t: " + concat(CStreamSettings::DataFormat::names(), CStreamSettings::DataFormat::count) + "\n";
	s += "data_type_sd\t\t: " + concat(CStreamSettings::DataType::names(), CStreamSettings::DataType::count) + "\n";
	s += "samples_limit_sd\t: An unsigned integer value. 0 - Disables write limit.\n";
	s += "adc_pass_mode\t\t: " + concat(CStreamSettings::PassMode::names(), CStreamSettings::PassMode::count) + "\n";
	s += "resolution\t\t: " + concat(CStreamSettings::Resolution::names(), CStreamSettings::Resolution::count) + "\n";
	s += "adc_decimation\t\t: An unsigned integer value: 1-65535.\n";
	s += "use_calib\t\t: " + concat(CStreamSettings::State::names(), CStreamSettings::State::count) + "\n";
	for (auto i = 1u; i <= 4; i++) {
		s += "channel_state_" + to_string(i) + "\t\t: " + concat(CStreamSettings::State::names(), CStreamSettings::State::count) + "\n";
		s += "channel_attenuator_" + to_string(i)
			 + "\t: " + concat(CStreamSettings::Attenuator::names(), CStreamSettings::Attenuator::count) + "\n";
		s += "channel_ac_dc_" + to_string(i) + "\t\t: " + concat(CStreamSettings::AC_DC::names(), CStreamSettings::AC_DC::count) + "\n";
	}
	s += "dac_rate\t\t: An unsigned integer value. Indicates the rate for signal generation. The maximum value should not be greater than "
		 "the base frequency of the FPGA.\n";
	s += "file_sd\t\t\t: Path to the file on the memory card in RP that will be used to generate the signal.\n";
	s += "file_type_sd\t\t: " + concat(CStreamSettings::DataFormat::names(), CStreamSettings::DataFormat::count) + "\n";
	s += "dac_pass_mode\t\t: " + concat(CStreamSettings::DACPassMode::names(), CStreamSettings::DACPassMode::count) + "\n";
	s += "repeat\t\t\t: " + concat(CStreamSettings::DACRepeat::names(), CStreamSettings::DACRepeat::count) + "\n";
	s += "repeatCount\t\t: An unsigned integer value.\n";
	for (auto i = 1u; i <= 2; i++) {
		s += "channel_gain_" + to_string(i) + "\t\t: " + concat(CStreamSettings::DACGain::names(), CStreamSettings::DACGain::count) + "\n";
	}

	s += "block_size\t\t: An unsigned integer value. The value must be less than the reserved memory in the system divided by 16. By "
		 "default, 32MB is allocated, the allowed maximum value is 2MB.\n";
	s += "adc_size\t\t: Minimum value 12 * (block_size + " + std::to_string(DataLib::sizeHeader()) + ")\n";
	s += "dac_size\t\t: Minimum value 12 * (block_size + " + std::to_string(DataLib::sizeHeader()) + ")\n";
	s += "gpio_size\t\t: Minimum value 12 * (block_size + " + std::to_string(DataLib::sizeHeader()) + ")\n";

	return s;
}

auto CStreamSettings::setDACDirPath(std::string &_path) -> void
{
	g_dirPath = _path;
}

auto CStreamSettings::getDACDirPath() -> std::string
{
#ifdef RP_PLATFORM
	if (g_dirPath == ""){
		if (!std::filesystem::exists(FILE_PATH)){
			std::filesystem::create_directories(FILE_PATH);
		}
		return FILE_PATH;
	}
	if (!std::filesystem::exists(g_dirPath)){
		std::filesystem::create_directories(g_dirPath);
	}
	return g_dirPath;
#else
	return ".";
#endif
}

auto CStreamSettings::getDACFiles() -> std::string
{
	std::string s = "";
	for (const auto &entry : std::filesystem::directory_iterator(getDACDirPath()))
		s += entry.path().filename().generic_string() + "\n";
	return s;
}
