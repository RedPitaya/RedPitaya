#ifndef SETTINGS_LIB_STREAMSETTINGS_H
#define SETTINGS_LIB_STREAMSETTINGS_H

#include "data_lib/network_header.h"
#include "enums.h"
#include <stdint.h>
#include <string>

class CStreamSettings
{
public:
	ENUM(DataFormat, WAV = 0, "wav", TDMS = 1, "tdms", BIN = 2, "bin")

	ENUM(State, OFF = 0, "Off", ON = 1, "On")

	ENUM(DataType, RAW = 0, "Raw", VOLT = 1, "Volt")

	ENUM(Resolution, BIT_8 = 0, "8 Bit", BIT_16 = 1, "16 Bit")

	ENUM(Attenuator, A_1_1 = 0, "1:1", A_1_20 = 1, "1:20")

	ENUM(AC_DC, AC = 0, "AC", DC = 1, "DC")

	ENUM(PassMode, NET = 0, "Network", FILE = 1, "File")

	ENUM(DACPassMode, DAC_NET = 0, "Network", DAC_FILE = 1, "File")

	ENUM(DACRepeat, DAC_REP_OFF = -1, "Off", DAC_REP_INF = -2, "Infinity", DAC_REP_ON = 0, "On")

	ENUM(DACGain, X1 = 0, "x1", X5 = 1, "x5")

	CStreamSettings();
	~CStreamSettings();
	CStreamSettings(const CStreamSettings &);
	CStreamSettings &operator=(const CStreamSettings &);

	static auto getHelp() -> std::string;

	auto resetDefault() -> void;
	auto copy(const CStreamSettings &) -> void;
	auto setValue(std::string key, std::string value) -> bool;
	auto getValue(std::string key) -> std::string;

	auto writeToFile(std::string _filename) -> bool;
	auto readFromFile(std::string _filename) -> bool;

	auto parseJson(const std::string &json) -> bool;
	auto toJson() const -> std::string;
	auto toString() const -> std::string;

	auto setADCSamples(uint64_t _samples) -> void;
	auto getADCSamples() const -> uint64_t;
	auto setADCFormat(DataFormat _format) -> void;
	auto getADCFormat() const -> DataFormat;
	auto setADCType(DataType _type) -> void;
	auto getADCType() const -> DataType;
	auto setADCPassMode(PassMode _mode) -> void;
	auto getADCPassMode() const -> PassMode;
	auto setADCChannels(uint8_t _channel, State _state) -> bool;
	auto getADCChannels(uint8_t _channel) const -> State;
	auto setADCResolution(Resolution _resolution) -> void;
	auto getADCResolution() const -> Resolution;
	auto setADCDecimation(uint32_t _decimation) -> bool;
	auto getADCDecimation() const -> uint32_t;
	auto setADCAttenuator(uint8_t _channel, Attenuator _attenuator) -> bool;
	auto getADCAttenuator(uint8_t _channel) const -> Attenuator;
	auto setADCAC_DC(uint8_t _channel, AC_DC _value) -> bool;
	auto getADCAC_DC(uint8_t _channel) const -> AC_DC;
	auto setADCCalibration(State _calibration) -> void;
	auto getADCCalibration() const -> State;

	auto setDACSpeed(uint32_t _value) -> bool;
	auto getDACSpeed() const -> uint32_t;
	auto setDACFile(std::string _value) -> void;
	auto getDACFile() const -> std::string;
	auto setDACFileType(DataFormat _value) -> bool;
	auto getDACFileType() const -> DataFormat;
	auto setDACGain(uint8_t _channel, DACGain _value) -> bool;
	auto getDACGain(uint8_t _channel) const -> DACGain;
	auto setDACPassMode(DACPassMode _value) -> void;
	auto getDACPassMode() const -> DACPassMode;
	auto setDACRepeat(DACRepeat _value) -> void;
	auto getDACRepeat() const -> DACRepeat;
	auto setDACRepeatCount(uint32_t _value) -> void;
	auto getDACRepeatCount() const -> uint32_t;

	auto setMemoryBlockSize(uint32_t size) -> void;
	auto getMemoryBlockSize() const -> uint32_t;
	auto setADCSize(uint32_t size) -> void;
	auto getADCSize() const -> uint32_t;
	auto setDACSize(uint32_t size) -> void;
	auto getDACSize() const -> uint32_t;
	auto setGPIOSize(uint32_t size) -> void;
	auto getGPIOSize() const -> uint32_t;

	static auto setDACDirPath(std::string &_path) -> void;
	static auto getDACDirPath() -> std::string;
	static auto getDACFiles() -> std::string;

private:
	CStreamSettings(CStreamSettings &&) = delete;
	CStreamSettings &operator=(CStreamSettings &&) = delete;

	struct ADCSettings
	{
		PassMode m_passMode = PassMode::NET;
		State m_channels[4] = {State::OFF, State::OFF, State::OFF, State::OFF};
		uint64_t m_samples = 0;
		DataFormat m_format = DataFormat::BIN;
		DataType m_dataType = DataType::RAW;
		Resolution m_resolution = Resolution::BIT_16;
		uint32_t m_decimation = 1;
		Attenuator m_attenuator[4] = {Attenuator::A_1_1, Attenuator::A_1_1, Attenuator::A_1_1, Attenuator::A_1_1};
		State m_useCalib = State::ON;
		AC_DC m_ac_dc[4] = {AC_DC::DC, AC_DC::DC, AC_DC::DC, AC_DC::DC};
	};

	struct MemorySettings
	{
		uint32_t m_memoryBlock = 0x10000; // data size 0x10000 + header size;
		uint32_t m_ADCSize = (0x10000 + DataLib::sizeHeader()) * 12;
		uint32_t m_DACSize = (0x10000 + DataLib::sizeHeader()) * 12;
		uint32_t m_GPIOSize = (0x10000 + DataLib::sizeHeader()) * 12;
	};

	struct DACSettings
	{
		std::string m_file = {""};
		DACGain m_gain[2] = {DACGain::X1, DACGain::X1};
		DataFormat m_file_type = DataFormat::WAV;
		DACPassMode m_passMode = DACPassMode::DAC_NET;
		DACRepeat m_repeat = DACRepeat::DAC_REP_OFF;
		uint64_t m_memoryUsage = 1024 * 1024;
		uint32_t m_repeatCount = 0;
		uint32_t m_rate = 125000000;
	};

	ADCSettings m_adcsettings;
	MemorySettings m_memorysettings;
	DACSettings m_dacsettings;
};

#endif
