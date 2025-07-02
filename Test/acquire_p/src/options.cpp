#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>

#include "common/version.h"
#include "options.h"

#define DEC_MAX 5

static constexpr uint32_t g_dec[DEC_MAX] = {1, 2, 4, 8, 16};

static constexpr char g_format_common[] =
    "\n"
    "Application for capturing data in split trigger mode.\n"
    "Usage: acquire_p [OPTION]... SIZE <DEC>\n"
    "    SIZE                Number of samples to acquire [1 - 16384].\n"
    "    DEC                 Decimation [1,2,4,8,16,17,18...65536] (default: 1). Valid values are from 1 to 65536\n"
    "\n";

static constexpr char g_format_common_settings[] =
    "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
    "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
    "  --bypass        -b    Bypass shaping filter in FPGA.\n"
    "  --version       -v    Print version info.\n"
    "  --help          -h    Print this message.\n"
    "  --hex           -x    Print value in hex.\n"
    "  --volt          -o    Print value in volt.\n"
    "  --calib         -c    Disable calibration parameters\n"
    "  --hk            -k    Reset houskeeping (Reset state for GPIO). Default: disabled\n"
    "  --debug         -g    Debug registers. Default: disabled\n"
    "  --offset              Offset relative to the trigger pointer [-16384 .. 16384]\n"
    "\n";

static constexpr char g_format_common_settings_gain_2ch[] =
    "  --att1=a              Use Channel 1 attenuator setting a [1, 20] (default: 1).\n"
    "  --att2=a              Use Channel 2 attenuator setting a [1, 20] (default: 1).\n"
    "\n";

static constexpr char g_format_common_settings_gain_4ch[] =
    "  --att1=a              Use Channel 1 attenuator setting a [1, 20] (default: 1).\n"
    "  --att2=a              Use Channel 2 attenuator setting a [1, 20] (default: 1).\n"
    "  --att3=a              Use Channel 3 attenuator setting a [1, 20] (default: 1).\n"
    "  --att4=a              Use Channel 4 attenuator setting a [1, 20] (default: 1).\n"
    "\n";

static constexpr char g_format_common_settings_trig_2ch[] =
    "  --tr_ch1=c      -1 c  Enable trigger for ch 1. Setting c use for channels [N (now), 1P, 1N, 2P, 2N, EP (ext channel), EN (ext channel)].\n"
    "  --tr_ch2=c      -2 c  Enable trigger for ch 2. Setting c use for channels [N (now), 1P, 1N, 2P, 2N, EP (ext channel), EN (ext channel)].\n"
    "  --tr_lev1=c           Set trigger level for ch 1 (default: 0).\n"
    "  --tr_lev2=c           Set trigger level for ch 2 (default: 0).\n"
    "\n";

static constexpr char g_format_common_settings_trig_4ch[] =
    "  --tr_ch1=c      -1 c  Enable trigger for ch 1. Setting c use for channels [N (now), 1P, 1N, 2P, 2N, 3P, 3N, 4P, 4N, EP (ext channel), EN (ext "
    "channel)].\n"
    "  --tr_ch2=c      -2 c  Enable trigger for ch 2. Setting c use for channels [N (now), 1P, 1N, 2P, 2N, 3P, 3N, 4P, 4N, EP (ext channel), EN (ext "
    "channel)].\n"
    "  --tr_ch3=c      -3 c  Enable trigger for ch 3. Setting c use for channels [N (now), 1P, 1N, 2P, 2N, 3P, 3N, 4P, 4N, EP (ext channel), EN (ext "
    "channel)].\n"
    "  --tr_ch4=c      -4 c  Enable trigger for ch 4. Setting c use for channels [N (now), 1P, 1N, 2P, 2N, 3P, 3N, 4P, 4N, EP (ext channel), EN (ext "
    "channel)].\n"
    "  --tr_lev1=c           Set trigger level for ch 1 (default: 0).\n"
    "  --tr_lev2=c           Set trigger level for ch 2 (default: 0).\n"
    "  --tr_lev3=c           Set trigger level for ch 3 (default: 0).\n"
    "  --tr_lev4=c           Set trigger level for ch 4 (default: 0).\n"
    "\n";

static constexpr char g_format_common_settings_ac_dc[] =
    "  --dc=c          -d c  Enable DC mode. Setting c use for channels [1, 2, B(Both channels)].\n"
    "\n";

static constexpr char g_format_common_settings_trig_ext_level[] =
    "  --tr_ext_level=c      Set trigger external level (default: 0).\n"
    "\n";

static constexpr char optstring_settings[] = "esbvxockgh";

static struct std::vector<option> long_options_settings = {
    /* These options set a flag. */
    {"equalization", no_argument, 0, 'e'}, {"shaping", no_argument, 0, 's'}, {"bypass", no_argument, 0, 'b'}, {"version", no_argument, 0, 'v'}, {"help", no_argument, 0, 'h'},
        {"hex", no_argument, 0, 'x'}, {"volt", no_argument, 0, 'o'}, {"calib", no_argument, 0, 'c'}, {"hk", no_argument, 0, 'k'}, {"debug", no_argument, 0, 'g'}, {
        "offset", required_argument, 0, 0
    }
};

static struct std::vector<option> long_options_settings_gain_2ch = {
    /* These options set a flag. */
    {"att1", required_argument, 0, 0}, { "att2", required_argument, 0, 0 }
};

static struct std::vector<option> long_options_settings_gain_4ch = {
    /* These options set a flag. */
    {"att1", required_argument, 0, 0}, {"att2", required_argument, 0, 0}, {"att3", required_argument, 0, 0}, { "att4", required_argument, 0, 0 }
};

static constexpr char optstring_settings_trig_2ch[] = "1:2:";

static struct std::vector<option> long_options_settings_trig_2ch = {
    /* These options set a flag. */
    {"tr_ch1", required_argument, 0, '1'}, {"tr_ch2", required_argument, 0, '2'}, {"tr_lev1", required_argument, 0, 0}, { "tr_lev2", required_argument, 0, 0 }
};

static constexpr char optstring_settings_trig_4ch[] = "1:2:3:4:";

static struct std::vector<option> long_options_settings_trig_4ch = {
    /* These options set a flag. */
    {"tr_ch1", required_argument, 0, '1'}, {"tr_ch2", required_argument, 0, '2'}, {"tr_ch3", required_argument, 0, '3'}, {"tr_ch4", required_argument, 0, '4'},
        {"tr_lev1", required_argument, 0, 0}, {"tr_lev2", required_argument, 0, 0}, {"tr_lev3", required_argument, 0, 0}, {
        "tr_lev4", required_argument, 0, 0
    }
};

static constexpr char optstring_settings_ac_dc[] = "d:";

static struct std::vector<option> long_options_settings_ac_dc = {
    /* These options set a flag. */
    { "dc", required_argument, 0, 'd' }
};

static struct std::vector<option> long_options_settings_ext_trig_level = {
    /* These options set a flag. */
    { "tr_ext_level", required_argument, 0, 0 }
};

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

int get_float(float* value, const char* str, const char* message, int min_value, int max_value) {
    try {
        if (str == NULL || *str == '\0')
            throw std::invalid_argument("null string");
        if (sscanf(str, "%f", value) != 1) {
            throw std::invalid_argument("invalid input string");
        }
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

int get_int(int* value, const char* str, const char* message, int min_value, int max_value) {
    try {
        if (str == NULL || *str == '\0')
            throw std::invalid_argument("null string");
        if (sscanf(str, "%d", value) != 1) {
            throw std::invalid_argument("invalid input string");
        }
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

int get_dc_mode(int* value, const char* str) {
    if (strncmp(str, "1", 1) == 0) {
        *value = 1;
        return 0;
    }

    if (strncmp(str, "2", 1) == 0) {
        *value = 2;
        return 0;
    }

    if ((strncmp(str, "B", 1) == 0) || (strncmp(str, "b", 1) == 0)) {
        *value = 3;
        return 0;
    }

    fprintf(stderr, "Unknown DC channel value: %s\n", str);
    return -1;
}

auto parseTrigger(char* value, int channels) -> int {
    if (strcmp(value, "N") == 0) {
        return RP_TRIG_SRC_NOW;
    }

    if (strcmp(value, "1P") == 0) {
        return RP_TRIG_SRC_CHA_PE;
    }

    if (strcmp(value, "1N") == 0) {
        return RP_TRIG_SRC_CHA_NE;
    }

    if (strcmp(value, "2P") == 0) {
        return RP_TRIG_SRC_CHB_PE;
    }

    if (strcmp(value, "2N") == 0) {
        return RP_TRIG_SRC_CHB_NE;
    }

    if (strcmp(value, "EP") == 0) {
        return RP_TRIG_SRC_EXT_PE;
    }

    if (strcmp(value, "EN") == 0) {
        return RP_TRIG_SRC_EXT_NE;
    }

    if (channels == 4) {
        if (strcmp(value, "3P") == 0) {
            return RP_TRIG_SRC_CHC_PE;
        }

        if (strcmp(value, "3N") == 0) {
            return RP_TRIG_SRC_CHC_NE;
        }

        if (strcmp(value, "4P") == 0) {
            return RP_TRIG_SRC_CHD_PE;
        }

        if (strcmp(value, "4N") == 0) {
            return RP_TRIG_SRC_CHD_NE;
        }
    }
    return -1;
}

/** Print usage information */
auto usage(Options& opt) -> void {
    fprintf(stderr, "Version: %s-%s\n", VERSION_STR, REVISION_STR);
    fprintf(stderr, "%s", opt.usage.c_str());
}

auto parse(int argc, char* argv[]) -> Options {
    Options opt;
    int option_index = 0;
    int ch = -1;

    opt.channels = getChannels();
    opt.is_ext_trig_lev = getIsExtTrigLevel();
    opt.is_ac_dc = getIsACDC();

    opt.usage = g_format_common;
    opt.opts = optstring_settings;
    opt.options.insert(opt.options.end(), long_options_settings.cbegin(), long_options_settings.cend());
    if (opt.channels == 2) {
        opt.usage += g_format_common_settings_gain_2ch;
        opt.usage += g_format_common_settings_trig_2ch;
        opt.opts += optstring_settings_trig_2ch;
        opt.options.insert(opt.options.end(), long_options_settings_gain_2ch.cbegin(), long_options_settings_gain_2ch.cend());
        opt.options.insert(opt.options.end(), long_options_settings_trig_2ch.cbegin(), long_options_settings_trig_2ch.cend());
    }

    if (opt.channels == 4) {
        opt.usage += g_format_common_settings_gain_4ch;
        opt.usage += g_format_common_settings_trig_4ch;
        opt.opts += optstring_settings_trig_4ch;
        opt.options.insert(opt.options.end(), long_options_settings_gain_4ch.cbegin(), long_options_settings_gain_4ch.cend());
        opt.options.insert(opt.options.end(), long_options_settings_trig_4ch.cbegin(), long_options_settings_trig_4ch.cend());
    }

    if (opt.is_ac_dc) {
        opt.usage += g_format_common_settings_ac_dc;
        opt.opts += optstring_settings_ac_dc;
        opt.options.insert(opt.options.end(), long_options_settings_ac_dc.cbegin(), long_options_settings_ac_dc.cend());
    }

    if (opt.is_ext_trig_lev) {
        opt.usage += g_format_common_settings_trig_ext_level;
        opt.options.insert(opt.options.end(), long_options_settings_ext_trig_level.cbegin(), long_options_settings_ext_trig_level.cend());
    }
    opt.usage += g_format_common_settings;
    opt.options.push_back({0, 0, 0, 0});

    if (argc < 2)
        return opt;

    opt.error = false;

    while ((ch = getopt_long(argc, argv, (char*)opt.opts.c_str(), (option*)opt.options.data(), &option_index)) != -1) {
        switch (ch) {

            case 0: {
                if (strcmp(opt.options[option_index].name, "offset") == 0) {
                    float offset = 0;
                    if (get_float(&offset, optarg, "Error get offset", -1024 * 16, 1024 * 16) != 0) {
                        opt.error = true;
                        return opt;
                    }
                    opt.offset = offset;
                    break;
                }

                if (strcmp(opt.options[option_index].name, "att1") == 0) {
                    if (strcmp(optarg, "1") == 0) {
                        opt.attenuator_mode[0] = RP_LOW;
                    } else if (strcmp(optarg, "20") == 0) {
                        opt.attenuator_mode[0] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                    break;
                }

                if (strcmp(opt.options[option_index].name, "att2") == 0) {
                    if (strcmp(optarg, "1") == 0) {
                        opt.attenuator_mode[1] = RP_LOW;
                    } else if (strcmp(optarg, "20") == 0) {
                        opt.attenuator_mode[1] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                    break;
                }

                if (strcmp(opt.options[option_index].name, "tr_lev1") == 0) {
                    float trig_level = 0;
                    if (get_float(&trig_level, optarg, "Error get trigger level", -10, 10) != 0) {
                        opt.error = true;
                        return opt;
                    }
                    opt.trigger_level[0] = trig_level;
                    break;
                }

                if (strcmp(opt.options[option_index].name, "tr_lev2") == 0) {
                    float trig_level = 0;
                    if (get_float(&trig_level, optarg, "Error get trigger level", -10, 10) != 0) {
                        opt.error = true;
                        return opt;
                    }
                    opt.trigger_level[1] = trig_level;
                    break;
                }

                if (strcmp(opt.options[option_index].name, "tr_ext_level") == 0) {
                    float trig_level = 0;
                    if (get_float(&trig_level, optarg, "Error get trigger level", 0, 10) != 0) {
                        opt.error = true;
                        return opt;
                    }
                    opt.trigger_level_ext = trig_level;
                    break;
                }

                if (opt.channels == 4) {
                    if (strcmp(opt.options[option_index].name, "att3") == 0) {
                        if (strcmp(optarg, "1") == 0) {
                            opt.attenuator_mode[2] = RP_LOW;
                        } else if (strcmp(optarg, "20") == 0) {
                            opt.attenuator_mode[2] = RP_HIGH;
                        } else {
                            fprintf(stderr, "Error key --get: %s\n", optarg);
                            opt.error = true;
                            return opt;
                        }
                        break;
                    }

                    if (strcmp(opt.options[option_index].name, "att4") == 0) {
                        if (strcmp(optarg, "1") == 0) {
                            opt.attenuator_mode[3] = RP_LOW;
                        } else if (strcmp(optarg, "20") == 0) {
                            opt.attenuator_mode[3] = RP_HIGH;
                        } else {
                            fprintf(stderr, "Error key --get: %s\n", optarg);
                            opt.error = true;
                            return opt;
                        }
                        break;
                    }

                    if (strcmp(opt.options[option_index].name, "tr_lev3") == 0) {
                        float trig_level = 0;
                        if (get_float(&trig_level, optarg, "Error get trigger level", -10, 10) != 0) {
                            opt.error = true;
                            return opt;
                        }
                        opt.trigger_level[2] = trig_level;
                        break;
                    }

                    if (strcmp(opt.options[option_index].name, "tr_lev4") == 0) {
                        float trig_level = 0;
                        if (get_float(&trig_level, optarg, "Error get trigger level", -10, 10) != 0) {
                            opt.error = true;
                            return opt;
                        }
                        opt.trigger_level[3] = trig_level;
                        break;
                    }
                }

                fprintf(stderr, "Error --%s: %s\n", opt.options[option_index].name, optarg);
                opt.error = true;
                return opt;
            }

            /* DC mode */
            case 'd': {
                int dc_mode;
                if (get_dc_mode(&dc_mode, optarg) != 0) {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                if (dc_mode == 1 || dc_mode == 3) {
                    opt.ac_dc_mode[0] = RP_DC;
                }
                if (dc_mode == 2 || dc_mode == 3) {
                    opt.ac_dc_mode[1] = RP_DC;
                }
                break;
            }

            case '1': {
                int trig = parseTrigger(optarg, opt.channels);
                if (trig != -1) {
                    opt.trigger_mode[0] = (rp_acq_trig_src_t)trig;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }

            case '2': {
                int trig = parseTrigger(optarg, opt.channels);
                if (trig != -1) {
                    opt.trigger_mode[1] = (rp_acq_trig_src_t)trig;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }

            case '3': {
                int trig = parseTrigger(optarg, opt.channels);
                if (trig != -1) {
                    opt.trigger_mode[2] = (rp_acq_trig_src_t)trig;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }

            case '4': {
                int trig = parseTrigger(optarg, opt.channels);
                if (trig != -1) {
                    opt.trigger_mode[3] = (rp_acq_trig_src_t)trig;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }

            case 's': {
                opt.enableShaping = true;
                break;
            }

            case 'e': {
                opt.enableEqualization = true;
                break;
            }

            case 'b': {
                opt.bypassFilter = true;
                break;
            }

            case 'v': {
                opt.showVersion = true;
                break;
            }

            case 'h': {
                opt.showHelp = true;
                break;
            }

            case 'x': {
                opt.showInHex = true;
                break;
            }

            case 'k': {
                opt.reset_hk = true;
                break;
            }

            case 'o': {
                opt.showInVolt = true;
                break;
            }

            case 'c': {
                opt.disableCalibration = true;
                break;
            }

            case 'g': {
                opt.enableDebug = true;
                break;
            }

            default: {
                fprintf(stderr, "[ERROR] Unknown parameter\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (opt.error || opt.showHelp || opt.showVersion)
        return opt;

    /* Acquisition size */
    uint32_t size = 0;
    if (optind < argc) {
        opt.dataSize = atoi(argv[optind]);
        if (size > ADC_BUFFER_SIZE) {
            fprintf(stderr, "Invalid SIZE: %s\n", argv[optind]);
            opt.error = true;
            return opt;
        }
    } else {
        fprintf(stderr, "SIZE parameter missing\n");
        opt.error = true;
        return opt;
    }
    optind++;

    /* Optional decimation */
    if (optind < argc) {
        opt.decimation = atoi(argv[optind]);

        if (opt.decimation <= 16) {
            auto findDec = false;
            for (int idx = 0; idx < DEC_MAX; idx++) {
                if (opt.decimation == g_dec[idx]) {
                    findDec = true;
                }
            }

            if (!findDec) {
                fprintf(stderr, "Invalid decimation DEC: %s\n", argv[optind]);
                opt.error = true;
                return opt;
            }
        }
        if (opt.decimation > 65536) {
            fprintf(stderr, "Invalid decimation DEC: %s\n", argv[optind]);
            opt.error = true;
            return opt;
        }
    }

    return opt;
}

uint8_t getChannels() {
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK) {
        FATAL("Can't get profile")
    }
    return c;
}

bool getIsExtTrigLevel() {
    bool c = 0;
    if (rp_HPGetIsExternalTriggerLevelPresent(&c) != RP_HP_OK) {
        FATAL("Can't get profile")
    }
    return c;
}

bool getIsSplitTriggers() {
    bool c = 0;
    if (rp_HPGetFastADCIsSplitTrigger(&c) != RP_HP_OK) {
        FATAL("Can't get profile")
    }
    return c;
}

bool getIsACDC() {
    bool c = 0;
    if (rp_HPGetFastADCIsAC_DC(&c) != RP_HP_OK) {
        FATAL("Can't get profile")
    }
    return c;
}