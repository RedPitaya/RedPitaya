#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace options {

typedef enum { RP_OPT_INT, RP_OPT_DOUBLE, RR_OPT_STRING } data_type_t;
typedef enum { RP_OPT_PARAM_MISSING, RP_OPT_PARAM_OPTIONAL, RP_OPT_PARAM_REQUIRED } param_mode_t;

typedef std::variant<int64_t, double, std::string> option_param_t;

struct ParamConfig {
    std::string name = {};
    std::string long_name = {};
    char short_name = {};
    param_mode_t has_param = RP_OPT_PARAM_MISSING;
    data_type_t param_type = RP_OPT_INT;
    std::pair<option_param_t, option_param_t> min_max = {};
    option_param_t default_value = {};
    std::string desc = {};
};

struct Options {
    std::map<std::string, option_param_t, std::less<>> params = {};
    bool error = false;
    auto getString(const std::string& name) -> const std::string*;
    auto getInt(const std::string& name) -> const int64_t*;
    auto getDouble(const std::string& name) -> const double*;
    auto isParam(const std::string& name) -> bool;
};

auto usage(char const* progName, const char* info) -> void;
auto printParams(const std::vector<ParamConfig>& opts) -> void;
auto parse(const std::vector<ParamConfig>& opts, int argc, char* argv[]) -> Options;

}  // namespace options

#endif