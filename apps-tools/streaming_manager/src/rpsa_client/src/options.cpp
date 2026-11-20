#include <getopt.h>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <strings.h>
#include <ctime>
#include <iostream>
#include <vector>

#include "help.h"
#include "options.h"
#include "settings_lib/stream_settings.h"

static struct option long_options_broadcast[] = {
    /* These options set a flag. */
    {"detect", no_argument, 0, 'd'},
    {"timeout", required_argument, 0, 't'},
    {"quiet", no_argument, 0, 'q'},
    {"reply-timeout", required_argument, 0, 'R'},  // long only
    {0, 0, 0, 0}};

static constexpr char optstring_broadcast[] = ":dt:q";

static struct option long_options_config[] = {
    /* These options set a flag. */
    {"config", no_argument, 0, 'c'},
    {"hosts", required_argument, 0, 'h'},
    {"get", required_argument, 0, 'g'},
    {"set", required_argument, 0, 's'},
    {"config_file", required_argument, 0, 'f'},
    {"config-file", required_argument, 0, 'f'},
    {"verbose", no_argument, 0, 'v'},
    {"write", no_argument, 0, 'w'},
    {"quiet", no_argument, 0, 'q'},
    {"reply-timeout", required_argument, 0, 'R'},  // long only
    {0, 0, 0, 0}};

static constexpr char optstring_config[] = ":ch:g:s:f:vwi:q";

static struct option long_options_remote[] = {
    /* These options set a flag. */
    {"remote", no_argument, 0, 'r'},
    {"hosts", required_argument, 0, 'h'},
    {"mode", required_argument, 0, 'm'},
    {"timeout", required_argument, 0, 't'},
    {"verbose", no_argument, 0, 'v'},
    {"quiet", no_argument, 0, 'q'},
    {"reply-timeout", required_argument, 0, 'R'},  // long only
    {0, 0, 0, 0}};

static constexpr char optstring_remote[] = ":rh:m:t:vq";

static struct option long_options_streaming[] = {
    /* These options set a flag. */
    {"streaming", no_argument, 0, 's'},
    {"hosts", required_argument, 0, 'h'},
    {"format", required_argument, 0, 'f'},
    {"dir", required_argument, 0, 'd'},
    {"limit", required_argument, 0, 'l'},
    {"mode", required_argument, 0, 'm'},
    {"timeout", required_argument, 0, 't'},
    {"verbose", no_argument, 0, 'v'},
    {"tdms_time", no_argument, 0, 'T'},  // long only
    {"tdms-time", no_argument, 0, 'T'},
    {"limit-time", required_argument, 0, 1001},  // long only
    {"limit-size", required_argument, 0, 1002},
    {"split", required_argument, 0, 1003},
    {"quiet", no_argument, 0, 'q'},
    {"reply-timeout", required_argument, 0, 'R'},  // long only
    {0, 0, 0, 0}};

static constexpr char optstring_streaming[] = ":sh:f:d:l:m:t:vq";

static struct option long_options_dac_streaming[] = {
    /* These options set a flag. */
    {"out_streaming", no_argument, 0, 'o'},
    {"out-streaming", no_argument, 0, 'o'},
    {"hosts", required_argument, 0, 'h'},
    {"format", required_argument, 0, 'f'},
    {"data", required_argument, 0, 'd'},
    {"repeat", required_argument, 0, 'r'},
    {"verbose", no_argument, 0, 'v'},
    {"quiet", no_argument, 0, 'q'},
    {"reply-timeout", required_argument, 0, 'R'},  // long only
    {0, 0, 0, 0}};

static constexpr char optstring_dac_streaming[] = ":oh:f:d:r:vq";

static struct option long_options_gpio[] = {
    {"gpio", no_argument, 0, 'G'},
    {"hosts", required_argument, 0, 'h'},
    {"in", required_argument, 0, 'I'},
    {"out", required_argument, 0, 'O'},
    {"format", required_argument, 0, 'f'},
    {"repeat", required_argument, 0, 'r'},
    {"gaps", no_argument, 0, 'P'},
    {"limit", required_argument, 0, 'l'},
    {"timeout", required_argument, 0, 't'},
    {"verbose", no_argument, 0, 'v'},
    {"quiet", no_argument, 0, 'q'},
    {"reply-timeout", required_argument, 0, 'R'},
    {0, 0, 0, 0}};

static constexpr char optstring_gpio[] = ":Gh:f:r:l:t:vq";

auto getTS(std::string suffix) -> std::string {
    using namespace std;
    using namespace std::chrono;

    auto timeNow = system_clock::now();
    auto ttime_t = system_clock::to_time_t(timeNow);

    auto ms = duration_cast<milliseconds>(timeNow.time_since_epoch()) % 1000;

    std::tm* ttm = std::localtime(&ttime_t);

    char time_buffer[32];
    strftime(time_buffer, sizeof(time_buffer), "%Y.%m.%d-%H.%M.%S", ttm);

    char final_buffer[64];
    snprintf(final_buffer, sizeof(final_buffer), "%s.%03lld%s", time_buffer, (long long)ms.count(), suffix.c_str());

    return std::string(final_buffer);
}

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

int get_int(int* value, const char* str, const char* message, int min_value, int max_value) {
    try {
        if (str == NULL || *str == '\0')
            throw std::invalid_argument("null string");
        auto checkstr = str;
        while (*checkstr) {
            if (*checkstr < '0' || *checkstr > '9')
                throw std::invalid_argument("invalid input string");
            ++checkstr;
        }
        *value = std::stoi(str);
    } catch (...) {
        fprintf(stderr, "%s: %s\n", message, str);
        return -1;
    }
    if (*value < min_value) {
        fprintf(stderr, "%s: %s\n", message, str);
        return -1;
    }
    if (*value > max_value) {
        fprintf(stderr, "%s: %s\n", message, str);
        return -1;
    }
    return 0;
}

// Decimal digits only, 64 bit without an overflow, within [min_value, max_value]
int get_uint64(uint64_t* value, const char* str, const char* message, uint64_t min_value, uint64_t max_value) {
    uint64_t v = 0;
    bool ok = str != NULL && *str != '\0';
    for (auto p = str; ok && *p; ++p) {
        const unsigned d = (unsigned)(*p - '0');
        if (*p < '0' || *p > '9' || v > (UINT64_MAX - d) / 10)
            ok = false;
        else
            v = v * 10 + d;
    }
    if (!ok || v < min_value || v > max_value) {
        fprintf(stderr, "%s: %s\n", message, str ? str : "");
        return -1;
    }
    *value = v;
    return 0;
}

int get_memory(int64_t* value, const char* str, const char* message) {
    try {
        if (str == NULL || *str == '\0')
            throw std::invalid_argument("null string");
        auto strLen = strlen(str);
        if (strLen >= 255)
            throw std::invalid_argument("invalid input string");
        char checkstr[255];
        strncpy(checkstr, str, strLen);
        bool subSuffix = false;
        int multipyMode = 1;
        if (str[strLen - 1] == 'M') {
            subSuffix = true;
            multipyMode = 1024 * 1024;
        } else if (str[strLen - 1] == 'k') {
            subSuffix = true;
            multipyMode = 1024;
        }
        if (subSuffix) {
            checkstr[strLen - 1] = 0;
        }
        int idx = 0;
        while (checkstr[idx]) {
            if (checkstr[idx] < '0' || checkstr[idx] > '9') {
                throw std::invalid_argument("invalid input string");
            }
            ++idx;
        }
        *value = std::stoi(str);
        *value *= multipyMode;
    } catch (...) {
        fprintf(stderr, "%s: %s\n", message, str);
        return -1;
    }
    return 0;
}

// "500ms", "5s", "2m" or a number in the unit of the option (unitMs: 1 for milliseconds, 1000 for
// seconds); false: not a time or above maxMs
// A size: bytes, or with K, M, G (1024 based), KB, MB, GB alike; capitals only
static auto parseSize(const char* str, uint64_t* bytes) -> bool {
    if (!str || !*str || str[0] == '-' || str[0] == '+')
        return false;
    char* end = nullptr;
    errno = 0;
    const double v = strtod(str, &end);
    if (end == str || errno || v < 0 || !std::isfinite(v))
        return false;
    // capitals: 5m is a time (minutes), 5M a size
    const std::string unit(end);
    double scale = 1;
    if (unit.empty() || unit == "B")
        scale = 1;
    else if (unit == "K" || unit == "KB")
        scale = 1024.0;
    else if (unit == "M" || unit == "MB")
        scale = 1024.0 * 1024;
    else if (unit == "G" || unit == "GB")
        scale = 1024.0 * 1024 * 1024;
    else
        return false;
    const double b = v * scale;
    if (b > 1.8e19)
        return false;
    *bytes = static_cast<uint64_t>(b);
    return true;
}

// A time of seconds: 90, 90s, 10m (or min), 2h; no unit means seconds
static auto parseDuration(const char* str, double* seconds) -> bool {
    if (!str || !*str || str[0] == '-' || str[0] == '+')
        return false;
    char* end = nullptr;
    errno = 0;
    const double v = strtod(str, &end);
    if (end == str || errno || v < 0 || !std::isfinite(v))
        return false;
    const std::string unit(end);
    double scale = 1;
    if (unit.empty() || unit == "s")
        scale = 1;
    else if (unit == "ms")
        scale = 0.001;
    else if (unit == "m" || unit == "min")
        scale = 60;
    else if (unit == "h")
        scale = 3600;
    else
        return false;
    *seconds = v * scale;
    return *seconds <= 366.0 * 24 * 3600;
}

static auto parseTime(const char* str, int64_t unitMs, int64_t maxMs, int64_t* ms) -> bool {
    if (!str || !*str)
        return false;
    char* end = nullptr;
    errno = 0;
    const double value = strtod(str, &end);  // 1.5s too
    if (end == str || value < 0 || errno || str[0] == '+' || str[0] == '-')
        return false;
    const std::string unit(end);
    int64_t scale = unitMs;
    if (unit == "ms")
        scale = 1;
    else if (unit == "s")
        scale = 1000;
    else if (unit == "m" || unit == "min")
        scale = 60000;
    else if (!unit.empty())
        return false;
    const double ms_ = value * scale;
    if (ms_ > static_cast<double>(maxMs))
        return false;
    *ms = static_cast<int64_t>(ms_ + 0.5);
    return true;
}

// the values of the options in any case: WAV and wav
static auto same(const char* a, const char* b) -> bool {
    return strcasecmp(a, b) == 0;
}

// the options every mode takes: --quiet, --reply-timeout
static auto commonOption(ClientOpt::Options& opt, int ch) -> bool {
    if (ch == 'q') {
        opt.quiet = true;
        return true;
    }
    if (ch == 'R') {
        int64_t ms = 0;
        if (!parseTime(optarg, 1000, 3600000, &ms) || ms == 0) {
            fprintf(stderr, "[ERROR] Wrong value of --reply-timeout: %s (seconds, or 500ms, 5s, 1m)\n", optarg);
            opt.mode = ClientOpt::Mode::ERROR_PARAM;
        } else {
            opt.reply_timeout_ms = static_cast<int>(ms);
        }
        return true;
    }
    return false;
}

auto check_ip(const std::string& value) -> bool {
    int octet[4], l;
    int r = sscanf(value.c_str(), "%d.%d.%d.%d%n", octet, octet + 1, octet + 2, octet + 3, &l);
    if (r != 4 || l != (int)value.length()) {
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (octet[i] < 0 || octet[i] >= 256) {
            return false;
        }
    }
    return true;
}

void remove_duplicates(std::vector<std::string>& vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

// An option getopt did not take: unknown, or without its value
static auto badOption(ClientOpt::Options& opt, char* argv[], int ch) -> ClientOpt::Options {
    const char* text = argv[optind - 1];
    if (ch == ':')
        fprintf(stderr, "[ERROR] Option %s needs a value\n", text);
    else if (optopt && !(text && text[0] == '-' && text[1] == '-'))
        fprintf(stderr, "[ERROR] Unknown option -%c\n", optopt);
    else
        fprintf(stderr, "[ERROR] Unknown option %s\n", text);
    opt.mode = ClientOpt::Mode::ERROR_PARAM;
    return opt;
}

auto ClientOpt::parse(int argc, char* argv[]) -> ClientOpt::Options {
    Options opt;
    opt.mode = Mode::ERROR_MODE;
    if (argc < 2)
        return opt;
    opterr = 0;  // the errors are told here, with the usage of the mode

    // --help [SECTION [TEXT]], --help=SECTION, -?: anywhere; after a mode key its section
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        if (a == "--help" || a == "-?" || a.rfind("--help=", 0) == 0) {
            opt.mode = Mode::SHOW_HELP;
            if (a.rfind("--help=", 0) == 0) {
                opt.help_topic = a.substr(7);
            } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                opt.help_topic = argv[i + 1];
                if (i + 2 < argc && argv[i + 2][0] != '-')
                    opt.help_filter = argv[i + 2];
            } else if (i > 1) {
                opt.help_topic = ClientHelp::topicOfKey(argv[1]);
            }
            return opt;
        }
        if (a == "--version") {
            opt.mode = Mode::SHOW_VERSION;
            return opt;
        }
    }
    // -q, -v, --reply-timeout may come before the mode: the first mode key goes to the front
    if (ClientHelp::topicOfKey(argv[1]).empty()) {
        for (int i = 2; i < argc; i++) {
            if (!ClientHelp::topicOfKey(argv[i]).empty()) {
                char* mode = argv[i];
                for (int k = i; k > 1; k--)
                    argv[k] = argv[k - 1];
                argv[1] = mode;
                break;
            }
        }
    }
    opt.help_topic = ClientHelp::topicOfKey(argv[1]);
    if (opt.help_topic.empty()) {
        fprintf(stderr, "[ERROR] Unknown mode %s: the first option is the mode (-d, -c, -r, -s, -o or -G; --help lists them)\n", argv[1]);
        opt.mode = Mode::ERROR_PARAM;
        return opt;
    }
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int ch = -1;
    if ((strcmp(argv[1], "-d") == 0) || (strcmp(argv[1], "--detect") == 0)) {
        opt.timeout = 5000;  // ms
        while ((ch = getopt_long(argc, argv, optstring_broadcast, long_options_broadcast, &option_index)) != -1) {
            switch (ch) {
                case 'd':
                    opt.mode = Mode::SEARCH;
                    break;

                case 't': {
                    int64_t ms = 0;
                    if (!parseTime(optarg, 1000, 3600000, &ms)) {
                        fprintf(stderr, "[ERROR] Wrong value of --timeout: %s (seconds, or 500ms, 5s, 1m)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = static_cast<int>(ms);  // the detect mode keeps milliseconds
                    break;
                }
                default:
                    if (commonOption(opt, ch)) {
                        if (opt.mode == Mode::ERROR_PARAM)
                            return opt;
                        break;
                    }
                    return badOption(opt, argv, ch);
            }
        }
        if (opt.mode != Mode::ERROR_MODE) {
            return opt;
        }
    }

    option_index = 0;
    if ((strcmp(argv[1], "-c") == 0) || (strcmp(argv[1], "--config") == 0)) {
        while ((ch = getopt_long(argc, argv, optstring_config, long_options_config, &option_index)) != -1) {
            switch (ch) {
                case 'c':
                    opt.mode = Mode::CONFIG;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'w':
                    opt.config_item_write = true;
                    break;

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto& s : opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'i': {
                    // NAME or NAME=VALUE, cut at the first "=": a value may hold more of them
                    const std::string arg = optarg;
                    const auto eq = arg.find('=');
                    ClientOpt::Options::ConfigItem item;
                    item.name = arg.substr(0, eq);
                    if (eq != std::string::npos) {
                        item.value = arg.substr(eq + 1);
                        item.write = true;
                    }
                    if (item.name.empty()) {
                        fprintf(stderr, "[ERROR] Wrong value of -i: %s (NAME or NAME=VALUE)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.config_items.push_back(item);
                    opt.config_item_name = item.name;
                    break;
                }

                case 'g': {
                    if (same(optarg, "V")) {
                        opt.conf_get = ConfGet::VERBOUS_JSON;
                    } else if (same(optarg, "V1")) {
                        opt.conf_get = ConfGet::VERBOUS_JSON_DATA;
                    } else if (same(optarg, "VV")) {
                        opt.conf_get = ConfGet::VERBOUS;
                    } else if (same(optarg, "F")) {
                        opt.conf_get = ConfGet::FILE;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --get: %s (V, V1, VV or F)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 's': {
                    if (same(optarg, "M")) {
                        opt.conf_set = ConfSet::MEMORY;
                    } else if (same(optarg, "F")) {
                        opt.conf_set = ConfSet::FILE;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --set: %s (M or F)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }



                case 'f': {
                    if (strcmp(optarg, "") != 0) {
                        opt.conf_file = optarg;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --config_file: '%s'\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                default:
                    if (commonOption(opt, ch)) {
                        if (opt.mode == Mode::ERROR_PARAM)
                            return opt;
                        break;
                    }
                    return badOption(opt, argv, ch);
            }
        }

        if (opt.mode != Mode::ERROR_MODE) {
            if (opt.mode == Mode::CONFIG && ((opt.conf_get == ConfGet::NONE && opt.conf_set == ConfSet::NONE) && opt.config_items.empty())) {
                fprintf(stderr, "[ERROR] The config mode needs -g, -s or -i\n");
                opt.mode = Mode::ERROR_PARAM;
            }
            return opt;
        }
    }

    option_index = 0;
    if ((strcmp(argv[1], "-r") == 0) || (strcmp(argv[1], "--remote") == 0)) {
        opt.timeout = 1000;  // ms, for start_stop
        while ((ch = getopt_long(argc, argv, optstring_remote, long_options_remote, &option_index)) != -1) {
            switch (ch) {
                case 'r':
                    opt.mode = Mode::REMOTE;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto& s : opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'm': {
                    if (same(optarg, "start")) {
                        opt.remote_mode = RemoteMode::START_WITH_ADC;
                    } else if (same(optarg, "stop")) {
                        opt.remote_mode = RemoteMode::STOP;
                    } else if (same(optarg, "start_stop")) {
                        opt.remote_mode = RemoteMode::START_STOP;
                    } else if (same(optarg, "start_dac")) {
                        opt.remote_mode = RemoteMode::START_DAC;
                    } else if (same(optarg, "stop_dac")) {
                        opt.remote_mode = RemoteMode::STOP_DAC;
                    } else if (same(optarg, "start_stop_dac")) {
                        opt.remote_mode = RemoteMode::START_STOP_DAC;
                    } else if (same(optarg, "start_gpio_in")) {
                        opt.remote_mode = RemoteMode::START_GPIO_IN;
                    } else if (same(optarg, "stop_gpio_in")) {
                        opt.remote_mode = RemoteMode::STOP_GPIO_IN;
                    } else if (same(optarg, "start_gpio_out")) {
                        opt.remote_mode = RemoteMode::START_GPIO_OUT;
                    } else if (same(optarg, "stop_gpio_out")) {
                        opt.remote_mode = RemoteMode::STOP_GPIO_OUT;
                    } else if (same(optarg, "watch")) {
                        opt.remote_mode = RemoteMode::WATCH;
                    } else if (same(optarg, "status")) {
                        opt.remote_mode = RemoteMode::STATUS;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --mode: %s (start, stop, start_stop, start_dac, stop_dac, start_stop_dac,\n        start_gpio_in, stop_gpio_in, start_gpio_out, stop_gpio_out, watch or status)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 't': {
                    int64_t ms = 0;
                    if (!parseTime(optarg, 1, 0xFFFFFFF, &ms)) {
                        fprintf(stderr, "[ERROR] Wrong value of --timeout: %s (milliseconds, or 500ms, 5s, 2m)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = static_cast<int>(ms);
                    break;
                }

                default:
                    if (commonOption(opt, ch)) {
                        if (opt.mode == Mode::ERROR_PARAM)
                            return opt;
                        break;
                    }
                    return badOption(opt, argv, ch);
            }
        }

        if (opt.mode != Mode::ERROR_MODE) {
            if (opt.mode == Mode::REMOTE && (opt.remote_mode == RemoteMode::NONE)) {
                fprintf(stderr, "[ERROR] The remote mode needs -m COMMAND\n");
                opt.mode = Mode::ERROR_PARAM;
            }
            return opt;
        }
    }

    option_index = 0;
    if ((strcmp(argv[1], "-s") == 0) || (strcmp(argv[1], "--streaming") == 0)) {
        opt.timeout = -1;
        while ((ch = getopt_long(argc, argv, optstring_streaming, long_options_streaming, &option_index)) != -1) {
            switch (ch) {
                case 's':
                    opt.mode = Mode::STREAMING;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'T':
                    opt.tdms_time = true;
                    break;

                case 1001: {
                    double s = 0;
                    if (!parseDuration(optarg, &s)) {
                        fprintf(stderr, "[ERROR] Wrong value of --limit-time: %s (seconds, or 90s, 10m, 2h)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.limit_time = s;
                    break;
                }
                case 1002: {
                    uint64_t b = 0;
                    if (!parseSize(optarg, &b) || b == 0) {
                        fprintf(stderr, "[ERROR] Wrong value of --limit-size: %s (bytes, or 500M, 2G)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.limit_size = b;
                    break;
                }
                case 1003: {
                    // a size with its unit (1G, at least 1M), or a time (5m; a number alone: seconds)
                    uint64_t b = 0;
                    double s = 0;
                    const char last = optarg[0] ? optarg[strlen(optarg) - 1] : 0;
                    const bool sized = last == 'B' || last == 'K' || last == 'M' || last == 'G';
                    if (sized && parseSize(optarg, &b) && b >= 1024 * 1024) {
                        opt.split_size = b;
                    } else if (!sized && parseDuration(optarg, &s) && s > 0) {
                        opt.split_time = s;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --split: %s (a size of 1M or more: 500M, 1G; or a time: 30s, 5m, 1h, 90)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto& s : opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'f': {
                    if (same(optarg, "tdms")) {
                        opt.streamign_type = StreamingType::TDMS;
                    } else if (same(optarg, "wav")) {
                        opt.streamign_type = StreamingType::WAV;
                    } else if (same(optarg, "csv")) {
                        opt.streamign_type = StreamingType::CSV;
                    } else if (same(optarg, "bin")) {
                        opt.streamign_type = StreamingType::BIN;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --format: %s (tdms, wav, csv or bin)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'm': {
                    if (same(optarg, "raw")) {
                        opt.save_type = SaveType::RAW;
                    } else if (same(optarg, "volt")) {
                        opt.save_type = SaveType::VOL;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --mode: %s (raw or volt)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }
                case 'l': {
                    uint64_t samp = 0;
                    if (get_uint64(&samp, optarg, "[ERROR] Wrong value of --limit (samples, 1-18446744073709551615)", 1, UINT64_MAX) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.samples = samp;
                    break;
                }

                case 't': {
                    int64_t ms = 0;
                    if (!parseTime(optarg, 1, 0xFFFFFFF, &ms)) {
                        fprintf(stderr, "[ERROR] Wrong value of --timeout: %s (milliseconds, or 500ms, 5s, 2m)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = static_cast<int>(ms);
                    break;
                }

                case 'd': {
                    if (strcmp(optarg, "") != 0) {
                        opt.save_dir = optarg;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --dir: '%s'\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                default:
                    if (commonOption(opt, ch)) {
                        if (opt.mode == Mode::ERROR_PARAM)
                            return opt;
                        break;
                    }
                    return badOption(opt, argv, ch);
            }
        }

        if (opt.mode != Mode::ERROR_MODE) {
            if (opt.mode == Mode::STREAMING && (opt.streamign_type == StreamingType::NONE)) {
                fprintf(stderr, "[ERROR] The streaming mode needs -f FORMAT\n");
                opt.mode = Mode::ERROR_PARAM;
            }
            return opt;
        }
    }

    option_index = 0;
    if ((strcmp(argv[1], "-o") == 0) || (strcmp(argv[1], "--out_streaming") == 0) || (strcmp(argv[1], "--out-streaming") == 0)) {
        opt.timeout = -1;
        while ((ch = getopt_long(argc, argv, optstring_dac_streaming, long_options_dac_streaming, &option_index)) != -1) {
            switch (ch) {
                case 'o':
                    opt.mode = Mode::STREAMING_DAC;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto& s : opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'f': {
                    if (same(optarg, "tdms")) {
                        opt.streamign_type = StreamingType::TDMS;
                    } else if (same(optarg, "wav")) {
                        opt.streamign_type = StreamingType::WAV;
                    } else if (same(optarg, "bin")) {
                        opt.streamign_type = StreamingType::BIN;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --format: %s (tdms, wav or bin)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'r': {
                    int repeat = 0;
                    if (same(optarg, "inf")) {
                        opt.dac_repeat = (int)RepeatDAC::INF;
                    } else if (get_int(&repeat, optarg, "[ERROR] Wrong value of --repeat (1-1000000 or inf)", 1, 1000000) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    } else {
                        opt.dac_repeat = repeat;
                    }

                    break;
                }

                case 'd': {
                    if (strcmp(optarg, "") != 0) {
                        opt.dac_file = optarg;
                    } else {
                        fprintf(stderr, "[ERROR] Wrong value of --data: '%s'\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                default:
                    if (commonOption(opt, ch)) {
                        if (opt.mode == Mode::ERROR_PARAM)
                            return opt;
                        break;
                    }
                    return badOption(opt, argv, ch);
            }
        }

        if (opt.mode != Mode::ERROR_MODE) {
            if (opt.mode == Mode::STREAMING_DAC && (opt.streamign_type == StreamingType::NONE)) {
                fprintf(stderr, "[ERROR] The DAC mode needs -f FORMAT\n");
                opt.mode = Mode::ERROR_PARAM;
            }
            if (opt.mode == Mode::STREAMING_DAC && opt.dac_file.empty()) {
                fprintf(stderr, "[ERROR] The DAC mode needs -d FILE\n");
                opt.mode = Mode::ERROR_PARAM;
            }
            return opt;
        }
    }

    option_index = 0;
    if ((strcmp(argv[1], "-G") == 0) || (strcmp(argv[1], "--gpio") == 0)) {
        opt.timeout = -1;
        while ((ch = getopt_long(argc, argv, optstring_gpio, long_options_gpio, &option_index)) != -1) {
            switch (ch) {
                case 'G':
                    opt.mode = Mode::GPIO;
                    break;
                case 'v':
                    opt.verbous = true;
                    break;
                case 'P':
                    opt.gpio_gaps = true;
                    break;
                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    for (auto& h : opt.hosts) {
                        if (!check_ip(h)) {
                            fprintf(stderr, "[ERROR] Wrong value of --hosts, not an IPv4 address: %s\n", h.c_str());
                            opt.mode = Mode::ERROR_PARAM;
                            return opt;
                        }
                    }
                    break;
                }
                case 'I':
                    opt.gpio_in_file = optarg;
                    break;
                case 'O':
                    opt.gpio_out_file = optarg;
                    break;
                case 'f':
                    if (!same(optarg, "bin") && !same(optarg, "csv") && !same(optarg, "vcd")) {
                        fprintf(stderr, "[ERROR] Wrong value of --format: %s (bin, csv or vcd)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.gpio_format = optarg;
                    break;
                case 'r': {
                    int repeat = 0;
                    if (same(optarg, "inf")) {
                        opt.dac_repeat = (int)RepeatDAC::INF;
                    } else if (get_int(&repeat, optarg, "[ERROR] Wrong value of --repeat (1-1000000 or inf)", 1, 1000000) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    } else {
                        opt.dac_repeat = repeat;
                    }
                    break;
                }
                case 'l': {
                    uint64_t samp = 0;
                    if (get_uint64(&samp, optarg, "[ERROR] Wrong value of --limit (samples, 1-18446744073709551615)", 1, UINT64_MAX) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.samples = samp;
                    break;
                }
                case 't': {
                    int64_t ms = 0;
                    if (!parseTime(optarg, 1, 0xFFFFFFF, &ms)) {
                        fprintf(stderr, "[ERROR] Wrong value of --timeout: %s (milliseconds, or 500ms, 5s, 2m)\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = static_cast<int>(ms);
                    break;
                }
                default:
                    if (commonOption(opt, ch)) {
                        if (opt.mode == Mode::ERROR_PARAM)
                            return opt;
                        break;
                    }
                    return badOption(opt, argv, ch);
            }
        }
        if (opt.mode == Mode::GPIO) {
            if (opt.gpio_in_file.empty() && opt.gpio_out_file.empty()) {
                fprintf(stderr, "[ERROR] The GPIO mode needs --in FILE, --out FILE or both\n");
                opt.mode = Mode::ERROR_PARAM;
            } else if (opt.hosts.size() > 1) {
                fprintf(stderr, "[ERROR] The GPIO mode takes one board\n");
                opt.mode = Mode::ERROR_PARAM;
            } else if (!opt.gpio_in_file.empty() && opt.gpio_format.empty()) {
                const auto dot = opt.gpio_in_file.find_last_of('.');
                const std::string ext = dot == std::string::npos ? "" : opt.gpio_in_file.substr(dot + 1);
                if (!same(ext.c_str(), "bin") && !same(ext.c_str(), "csv") && !same(ext.c_str(), "vcd")) {
                    fprintf(stderr, "[ERROR] The format of %s: give it with -f bin|csv|vcd or as the extension\n", opt.gpio_in_file.c_str());
                    opt.mode = Mode::ERROR_PARAM;
                }
            }
        }
        return opt;
    }

    return opt;
}

auto time_point_to_string(std::chrono::system_clock::time_point& tp) -> std::string {
    using namespace std;
    using namespace std::chrono;

    auto ttime_t = system_clock::to_time_t(tp);
    auto tp_sec = system_clock::from_time_t(ttime_t);
    milliseconds ms = duration_cast<milliseconds>(tp - tp_sec);

    std::tm* ttm = localtime(&ttime_t);

    char date_time_format[] = "%Y.%m.%d-%H.%M.%S";

    char time_str[] = "yyyy.mm.dd.HH-MM.SS.fff";

    strftime(time_str, strlen(time_str), date_time_format, ttm);

    string result(time_str);
    result.append(".");
    result.append(to_string(ms.count()));

    return result;
}
