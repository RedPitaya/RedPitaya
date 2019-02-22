#pragma once

#include <string>

struct cli_args_t {
    float freq_min = 0.;
    float freq_max = 62500000.;
    int count = 1;
    bool average_for_10 = true;
    bool csv = false;
    bool csv_limit = false;
    bool test = false;
    bool help = false;
};

std::string cli_help_string();

// Returns false when the parser failure
bool cli_parse_args(int argc, char * const argv[], cli_args_t &out_args);
