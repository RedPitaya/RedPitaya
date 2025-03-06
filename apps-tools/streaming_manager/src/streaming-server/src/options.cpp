#include <getopt.h>
#include <cstring>
#include <vector>

#ifdef RP_PLATFORM
#include "rp_hw-profiles.h"
#endif

#ifndef _WIN32
#define LOG_EMERG 0   /* system is unusable */
#define LOG_ALERT 1   /* action must be taken immediately */
#define LOG_CRIT 2    /* critical conditions */
#define LOG_ERR 3     /* error conditions */
#define LOG_WARNING 4 /* warning conditions */
#define LOG_NOTICE 5  /* normal but significant condition */
#define LOG_INFO 6    /* informational */
#define LOG_DEBUG 7   /* debug-level messages */
#endif

#include "options.h"

static struct option long_options[] = {
    /* These options set a flag. */
    {"background", no_argument, 0, 'b'},
    {"file", required_argument, 0, 'f'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};

static constexpr char optstring[] = "bf:hv";

std::vector<std::string> ClientOpt::split(const std::string& s, char seperator) {
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
        printWithLog(LOG_ERR, stderr, "%s: %s\n", message, str);
        return -1;
    }
    if (*value < min_value) {
        printWithLog(LOG_ERR, stderr, "%s: %s\n", message, str);
        return -1;
    }

    if (*value > max_value) {
        printWithLog(LOG_ERR, stderr, "%s: %s\n", message, str);
        return -1;
    }
    return 0;
}

auto ClientOpt::getADCChannels() -> uint8_t {
    uint8_t c = 0;
#ifdef RP_PLATFORM
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK) {
        FATAL("Can't get fast ADC channels count")
    }
    if (c > MAX_ADC_CHANNELS) {
        FATAL("The number of channels is more than allowed")
    }
#else
    c = 2;  // Emulation
#endif
    return c;
}

auto ClientOpt::getADCBits() -> uint8_t {
    uint8_t c = 0;
#ifdef RP_PLATFORM
    if (rp_HPGetFastADCBits(&c) != RP_HP_OK) {
        FATAL("Can't get fast ADC channels count")
    }
    if (c > 16) {
        FATAL("The number of bits is more than allowed")
    }
#else
    c = 16;  // Emulation
#endif
    return c;
}

auto ClientOpt::getDACChannels() -> uint8_t {
    uint8_t c = 0;
#ifdef RP_PLATFORM
    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK) {
        FATAL("Can't get fast DAC channels count")
    }
    if (c > MAX_DAC_CHANNELS) {
        FATAL("The number of channels is more than allowed")
    }
#else
    c = 2;  // Emulation
#endif
    return c;
}

auto ClientOpt::getDACRate() -> uint32_t {
    uint32_t c = 0;
#ifdef RP_PLATFORM
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK) {
        FATAL("Can't get fast DAC channels count")
    }
#else
    c = 10e6;  // Emulation
#endif
    return c;
}

auto ClientOpt::getADCRate() -> uint32_t {
    uint32_t c = 0;
#ifdef RP_PLATFORM
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK) {
        FATAL("Can't get fast ADC channels count")
    }
#else
    c = 10e6;  // Emulation
#endif
    return c;
}

auto ClientOpt::getModel() -> ClientOpt::models_t {
#ifdef RP_PLATFORM
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        FATAL("Can't get board model")
    }

    switch (c) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return RP_125_14;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return RP_125_14;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return RP_125_14_4CH;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            return RP_250_12;
        case STEM_250_12_120:
            return RP_250_12;
        default:
            FATAL("Can't get board model")
    }
#endif
    return RP_125_14;
}

auto ClientOpt::getBroadcastModel() -> broadcast_lib::EModel {
#ifdef RP_PLATFORM
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        FATAL("Can't get board model")
    }

    switch (c) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
            return broadcast_lib::EModel::RP_125_14;
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return broadcast_lib::EModel::RP_125_14_Z20;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return broadcast_lib::EModel::RP_122_16;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return broadcast_lib::EModel::RP_125_4CH;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            return broadcast_lib::EModel::RP_250_12;
        case STEM_250_12_120:
            return broadcast_lib::EModel::RP_250_12;

        default:
            FATAL("Can't get board model")
    }
#endif
    return broadcast_lib::EModel::RP_125_14;
}

/** Print usage information */
auto ClientOpt::usage(char const* progName) -> void {
#ifdef _WIN32
    auto arr = split(std::string(progName), '\\');
#else
    auto arr = split(std::string(progName), '/');
#endif

    std::string name = "";
    if (arr.size() > 0)
        name = arr[arr.size() - 1];
    const char* format =
        "Usage: \n"
        "\t%s [-b] [-f PATH] [-p PORT] [-s PORT] [-v]\n"
        "\t%s [--background] [--file=PATH] [--port=PORT] [--search_port=PORT] [--verbose]\n"
        "\n"
        "\t--background          -b        Run service in background.\n"
        "\t--file=PATH           -f FILE   Path to configuration file.\n"
        "\t                                By default uses the config file /root/.config/redpitaya/apps/streaming/streaming_config.json.\n"
        "\t--verbose             -v        Displays information.\n"
        "\n"
        "\t Example:\n"
        "\t\t%s -b -f /root/.streaming_config_new.json\n";

    auto n = name.c_str();
    printWithLog(LOG_INFO, stdout, format, n, n);
}

auto ClientOpt::parse(int argc, char* argv[]) -> ClientOpt::Options {
    Options opt;
    if (argc < 2)
        return opt;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int ch = -1;

    while ((ch = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
        switch (ch) {
            case 'b':
                opt.background = true;
                break;

            case 'h': {
                usage(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            }

            case 'v':
                opt.verbose = true;
                break;

            case 'f': {
                if (strcmp(optarg, "") != 0) {
                    opt.conf_file = optarg;
                } else {
                    printWithLog(LOG_ERR, stderr, "[ERROR] key --file: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            }

            default: {
                printWithLog(LOG_ERR, stderr, "[ERROR] Unknown parameter\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return opt;
}
