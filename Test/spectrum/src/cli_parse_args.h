#pragma once

#include <string>
#include "rp_hw-profiles.h"
#include "math/rp_dsp.h"

uint32_t getMaxFreqRate();

#define F_MAX (getMaxFreqRate())

struct cli_args_t {
    float freq_min = 0.;
    float freq_max = F_MAX;
    int count = 1;
    bool average_for_10 = true;
    bool csv = false;
    bool csv_limit = false;
    bool help = false;
    bool test = false;
    rp_dsp_api::window_mode_t wm = rp_dsp_api::HANNING;
};

std::string cli_help_string();

// Returns false when the parser failure
bool cli_parse_args(int argc, char * const argv[], cli_args_t &out_args);