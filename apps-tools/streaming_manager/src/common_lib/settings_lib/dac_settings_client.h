#ifndef SETTINGS_LIB_DACSETTINGS_CLIENT_H
#define SETTINGS_LIB_DACSETTINGS_CLIENT_H

#include <string>
#include "stream_settings.h"

struct DACSettingsClient
{
	std::string host = "";
	std::string dac_file = "";
	CStreamSettings::DataFormat file_type = CStreamSettings::DataFormat::WAV;
	CStreamSettings::DACRepeat dac_repeat_mode = CStreamSettings::DACRepeat::DAC_REP_OFF;
	int64_t dac_repeat = 0;
	int64_t dac_memory = 0;
	int32_t dac_speed = 0;
	bool verbous = false;
};

#endif
