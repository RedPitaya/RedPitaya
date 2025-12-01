#include <getopt.h>
#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "common/version.h"
#include "options.h"

using namespace options;

std::vector<std::string> split(const std::string& s, char seperator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while ((pos = s.find(seperator, pos)) != std::string::npos) {
        std::string substring(s.substr(prev_pos, pos - prev_pos));
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos - prev_pos));  // Last word
    return output;
}

template <typename T>
int get_value(T* value, const char* str, const std::string& message, T min_value, T max_value) {
    try {
        if (str == NULL || *str == '\0')
            throw std::invalid_argument("null string");

        const char* format = nullptr;
        const char* formatHex = nullptr;

        if constexpr (std::is_same_v<T, int>) {
            format = "%d";
        } else if constexpr (std::is_same_v<T, float>) {
            format = "%f";
        } else if constexpr (std::is_same_v<T, double>) {
            format = "%lf";
        } else if constexpr (std::is_same_v<T, char>) {
            format = "%c";
        } else if constexpr (std::is_same_v<T, int8_t>) {
            format = "%" SCNd8;
            formatHex = "0x%" SCNx8;
        } else if constexpr (std::is_same_v<T, uint8_t>) {
            format = "%" SCNu8;
            formatHex = "0x%" SCNx8;
        } else if constexpr (std::is_same_v<T, int16_t>) {
            format = "%" SCNd16;
            formatHex = "0x%" SCNx16;
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            format = "%" SCNu16;
            formatHex = "0x%" SCNx16;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            format = "%" SCNd32;
            formatHex = "0x%" SCNx32;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            format = "%" SCNu32;
            formatHex = "0x%" SCNx32;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            format = "%" SCNd64;
            formatHex = "0x%" SCNx64;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            format = "%" SCNu64;
            formatHex = "0x%" SCNx64;
        } else if constexpr (std::is_same_v<T, std::string>) {
            char buffer[256];
            if (sscanf(str, "%255s", buffer) != 1) {
                throw std::runtime_error("Failed to parse string");
            }
            *value = std::move(std::string(buffer));
            return 0;
        } else {
            static_assert(std::is_same_v<T, void>, "Unsupported type for sscanf");
        }

        if (!formatHex || sscanf(str, formatHex, value) != 1) {
            if (sscanf(str, format, value) != 1) {
                throw std::runtime_error("Failed to parse value");
            }
        }

    } catch (...) {
        fprintf(stderr, "%s: %s\n", message.c_str(), str);
        return -1;
    }
    if (*value < min_value) {
        fprintf(stderr, "%s: %s\n", message.c_str(), str);
        return -1;
    }
    if (*value > max_value) {
        fprintf(stderr, "%s: %s\n", message.c_str(), str);
        return -1;
    }
    return 0;
}

auto options::Options::getString(const std::string& name) -> const std::string* {
    if (auto search = params.find(name); search != params.end()) {
        auto& val = search->second;
        if (const std::string* pval = std::get_if<std::string>(&val))
            return pval;
    }
    return NULL;
}

auto options::Options::getInt(const std::string& name) -> const int64_t* {
    if (auto search = params.find(name); search != params.end()) {
        auto& val = search->second;
        if (const int64_t* pval = std::get_if<int64_t>(&val))
            return pval;
    }
    return NULL;
}

auto options::Options::getDouble(const std::string& name) -> const double* {
    if (auto search = params.find(name); search != params.end()) {
        auto& val = search->second;
        if (const double* pval = std::get_if<double>(&val))
            return pval;
    }
    return NULL;
}

auto options::Options::isParam(const std::string& name) -> bool {
    return params.count(name.c_str());
}

/** Print usage information */
auto options::usage(char const* progName, const char* info) -> void {

    auto arr = split(std::string(progName), '/');

    std::string name = "";
    if (arr.size() > 0)
        name = arr[arr.size() - 1];

    auto n = name.c_str();

    fprintf(stderr, "Version: %s-%s\n%s %s\n", VERSION_STR, REVISION_STR, n, info);
}

auto options::printParams(const std::vector<ParamConfig>& opts) -> void {
    auto fixed_width = [](const std::string& input, int width) -> std::string {
        std::ostringstream oss;
        oss << std::left << std::setw(width) << std::setfill(' ') << input;
        std::string result = oss.str();
        if (result.length() > static_cast<size_t>(width)) {
            result.resize(width);
        }
        return result;
    };

    std::string help = "";
    std::string long_param = "";
    for (auto& item : opts) {
        long_param = item.long_name;

        switch (item.has_param) {
            case RP_OPT_PARAM_MISSING:
                break;
            case RP_OPT_PARAM_OPTIONAL:
                long_param += item.has_param ? "[=X]" : "";
                break;
            case RP_OPT_PARAM_REQUIRED:
                long_param += item.has_param ? "=X" : "";
                break;
        }

        long_param = fixed_width(long_param, 20);
        help += "  --" + long_param;

        if (item.short_name != 0) {
            help += " -" + std::string(1, item.short_name);
            switch (item.has_param) {
                case RP_OPT_PARAM_MISSING:
                    help += "      ";
                    break;
                case RP_OPT_PARAM_OPTIONAL:
                    help += " [X]  ";
                    break;
                case RP_OPT_PARAM_REQUIRED:
                    help += " X    ";
                    break;
            }
        } else {
            help += "   ";
            help += "      ";
        }

        help += item.desc + "\n";
    }
    fprintf(stderr, "%s", help.c_str());
}

auto options::parse(const std::vector<ParamConfig>& opts, int argc, char* argv[]) -> Options {
    Options ret_opt;
    if (argc < 2)
        return ret_opt;
    int option_index = 0;
    int ch = -1;

    ret_opt.error = false;

    struct std::vector<option> long_opts;
    std::string short_opt;

    for (auto& item : opts) {
        int arg = 0;
        switch (item.has_param) {
            case RP_OPT_PARAM_MISSING:
                arg = no_argument;
                break;
            case RP_OPT_PARAM_OPTIONAL:
                arg = optional_argument;
                break;
            case RP_OPT_PARAM_REQUIRED:
                arg = required_argument;
                break;
        }

        long_opts.push_back({item.long_name.c_str(), arg, 0, item.short_name});
        if (item.short_name != 0) {
            short_opt += std::string(1, item.short_name);
            switch (item.has_param) {
                case RP_OPT_PARAM_MISSING:
                    break;
                case RP_OPT_PARAM_OPTIONAL:
                    short_opt += "::";
                    break;
                case RP_OPT_PARAM_REQUIRED:
                    short_opt += ":";
                    break;
            }
        }
    }
    long_opts.push_back({0, 0, 0, 0});
    opterr = 0;
    while ((ch = getopt_long(argc, argv, short_opt.c_str(), long_opts.data(), &option_index)) != -1) {
        if (ch != ':' && ch != '?') {

            ParamConfig item;
            if (ch == 0)
                item = opts[option_index];
            else {
                for (auto& itm : opts) {
                    if (itm.short_name == ch) {
                        item = itm;
                        break;
                    }
                }
            }

            if (item.name == "") {
                fprintf(stderr, "FATAL ERROR!\n");
                ret_opt.error = true;
                return ret_opt;
            }

            switch (item.has_param) {
                case RP_OPT_PARAM_MISSING:
                    ret_opt.params[item.name] = {};
                    break;
                case RP_OPT_PARAM_OPTIONAL:
                    if (optarg) {
                        switch (item.param_type) {
                            case RP_OPT_INT: {
                                int64_t value_i = 0;
                                if (get_value<int64_t>(&value_i, optarg, "Error get parameter for key: " + item.long_name, std::get<int64_t>(item.min_max.first),
                                                       std::get<int64_t>(item.min_max.second)) == 0) {
                                    ret_opt.params[item.name] = value_i;
                                } else {
                                    ret_opt.error = true;
                                    return ret_opt;
                                }
                                break;
                            }
                            case RP_OPT_DOUBLE: {
                                double value_d = 0;
                                if (get_value<double>(&value_d, optarg, "Error get parameter for key: " + item.long_name, std::get<double>(item.min_max.first),
                                                      std::get<double>(item.min_max.second)) == 0) {
                                    ret_opt.params[item.name] = value_d;
                                } else {
                                    ret_opt.error = true;
                                    return ret_opt;
                                }
                                break;
                            }
                            case RR_OPT_STRING: {
                                std::string value_s = "";
                                if (get_value<std::string>(&value_s, optarg, "Error get parameter for key: " + item.long_name, std::get<std::string>(item.min_max.first),
                                                           std::get<std::string>(item.min_max.second)) == 0) {
                                    ret_opt.params[item.name] = value_s;
                                } else {
                                    ret_opt.error = true;
                                    return ret_opt;
                                }
                                break;
                            }
                        }
                    } else {
                        ret_opt.params[item.name] = item.default_value;
                    }
                    break;
                case RP_OPT_PARAM_REQUIRED:
                    ret_opt.params[item.name] = {};
                    if (optarg) {
                        switch (item.param_type) {
                            case RP_OPT_INT: {
                                int64_t value_i = 0;
                                if (get_value<int64_t>(&value_i, optarg, "Error get parameter for key: " + item.long_name, std::get<int64_t>(item.min_max.first),
                                                       std::get<int64_t>(item.min_max.second)) == 0) {
                                    ret_opt.params[item.name] = value_i;
                                } else {
                                    ret_opt.error = true;
                                    return ret_opt;
                                }
                                break;
                            }
                            case RP_OPT_DOUBLE: {
                                double value_d = 0;
                                if (get_value<double>(&value_d, optarg, "Error get parameter for key: " + item.long_name, std::get<double>(item.min_max.first),
                                                      std::get<double>(item.min_max.second)) == 0) {
                                    ret_opt.params[item.name] = value_d;
                                } else {
                                    ret_opt.error = true;
                                    return ret_opt;
                                }
                                break;
                            }
                            case RR_OPT_STRING: {
                                std::string value_s = "";
                                if (get_value<std::string>(&value_s, optarg, "Error get parameter for key: " + item.long_name, std::get<std::string>(item.min_max.first),
                                                           std::get<std::string>(item.min_max.second)) == 0) {
                                    ret_opt.params[item.name] = value_s;
                                } else {
                                    ret_opt.error = true;
                                    return ret_opt;
                                }
                                break;
                            }
                        }
                    } else {
                        fprintf(stderr, "Error. Missing value for parameter: %s\n", item.long_name.c_str());
                        ret_opt.error = true;
                        return ret_opt;
                    }
                    break;
            }
        }
        if (ch == '?') {
            fprintf(stderr, "Error: Invalid option\n");
            ret_opt.error = true;
            return ret_opt;
        }

        if (ch == ':') {
            fprintf(stderr, "Error: Invalid parameter\n");
            ret_opt.error = true;
            return ret_opt;
        }
    }

    return ret_opt;
}