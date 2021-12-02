#pragma once
#include <string>
#include <vector>
#include "stream_settings.h"

struct DacSettings {
    std::string                 host = "";
    std::string                 port = "";
    std::string                 config_port = "";
    std::string                 dac_file = "";
    CStreamSettings::DataFormat file_type = CStreamSettings::UNDEF;
    CStreamSettings::DACRepeat  dac_repeat_mode = CStreamSettings::DAC_REP_OFF;
    int64_t                     dac_repeat = 0;
    int64_t                     dac_memory = 0;
    bool                        verbous    = false;

    static auto readFromFile(std::string _filename) -> std::vector<DacSettings>;
};

