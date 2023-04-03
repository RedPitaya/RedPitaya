#include <iostream>
#include <getopt.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <chrono>

#include "version.h"
#include "options.h"

#define DEC_MAX 5

static constexpr uint32_t g_dec[DEC_MAX] = { 1,  2,  4,  8,  16 };


static constexpr char optstring_250_12[64] = "esx1:2:d:vht:l:orcka";
static struct option long_options_250_12[32] = {
        /* These options set a flag. */
        {"equalization", no_argument,       0, 'e'},
        {"shaping",      no_argument,       0, 's'},
        {"atten1",       required_argument, 0, '1'},
        {"atten2",       required_argument, 0, '2'},
        {"dc",           required_argument, 0, 'd'},
        {"tr_ch",        required_argument, 0, 't'},
        {"tr_level",     required_argument, 0, 'l'},
        {"version",      no_argument,       0, 'v'},
        {"help",         no_argument,       0, 'h'},
        {"hex",          no_argument,       0, 'x'},
        {"volt",         no_argument,       0, 'o'},
        {"no_reg",       no_argument,       0, 'r'},
        {"calib",        no_argument,       0, 'c'},
        {"hk",           no_argument,       0, 'k'},
        {"axi",          no_argument,       0, 'a'},
        {0, 0, 0, 0}
};

static constexpr char g_format_250_12[2048] =
        "\n"
        "Usage: %s [OPTION]... SIZE <DEC>\n"
        "\n"
        "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
        "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
        "  --atten1=a      -1 a  Use Channel 1 attenuator setting a [1, 20] (default: 1).\n"
        "  --atten2=a      -2 a  Use Channel 2 attenuator setting a [1, 20] (default: 1).\n"
        "  --dc=c          -d c  Enable DC mode. Setting c use for channels [1, 2, B(Both channels)].\n"
        "                        By default, AC mode is turned on.\n"
        "  --tr_ch=c       -t c  Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N, EP (external channel), EN (external channel)].\n"
        "                        P - positive edge, N -negative edge. By default trigger no set\n"
        "  --tr_level=c    -l c  Set trigger level (default: 0).\n"
        "  --version       -v    Print version info.\n"
        "  --help          -h    Print this message.\n"
        "  --hex           -x    Print value in hex.\n"
        "  --volt          -o    Print value in volt.\n"
        "  --no_reg        -r    Disable load registers config (XML) for DAC and ADC.\n"
        "  --calib         -c    Disable calibration parameters\n"
        "  --hk            -k    Reset houskeeping (Reset state for GPIO). Default: disabled\n"
        "  --axi           -a    Enable AXI interface. Also enable housekeeping reset. Default: disabled\n"
        "    SIZE                Number of samples to acquire [0 - %u].\n"
        "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
        "\n";


static constexpr char optstring_125_14[64] = "esx1:2:vht:l:ocka";
static struct option long_options_125_14[32] = {
        /* These options set a flag. */
        {"equalization", no_argument,       0, 'e'},
        {"shaping",      no_argument,       0, 's'},
        {"gain1",        required_argument, 0, '1'},
        {"gain2",        required_argument, 0, '2'},
        {"tr_ch",        required_argument, 0, 't'},
        {"tr_level",     required_argument, 0, 'l'},
        {"version",      no_argument,       0, 'v'},
        {"help",         no_argument,       0, 'h'},
        {"hex",          no_argument,       0, 'x'},
        {"volt",         no_argument,       0, 'o'},
        {"calib",        no_argument,       0, 'c'},
        {"hk",           no_argument,       0, 'k'},
        {"axi",          no_argument,       0, 'a'},
        {0, 0, 0, 0}
};

static constexpr char g_format_125_14[2048] =
        "\n"
        "Usage: %s [OPTION]... SIZE <DEC>\n"
        "\n"
        "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
        "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
        "  --gain1=g       -1 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
        "  --gain2=g       -2 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
        "  --tr_ch=c       -t c  Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N, EP (external channel), EN (external channel)].\n"
        "                        P - positive edge, N -negative edge. By default trigger no set\n"
        "  --tr_level=c    -l c  Set trigger level (default: 0).\n"
        "  --version       -v    Print version info.\n"
        "  --help          -h    Print this message.\n"
        "  --hex           -x    Print value in hex.\n"
        "  --volt          -o    Print value in volt.\n"
        "  --calib         -c    Disable calibration parameters\n"
        "  --hk            -k    Reset houskeeping (Reset state for GPIO). Default: disabled\n"
        "  --axi           -a    Enable AXI interface. Also enable housekeeping reset. Default: disabled\n"
        "    SIZE                Number of samples to acquire [0 - %u].\n"
        "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
        "\n";


static constexpr char optstring_125_14_4ch[64] = "esx1:2:3:4:vht:l:ock";
static struct option long_options_125_14_4ch[32] = {
        /* These options set a flag. */
        {"equalization", no_argument,       0, 'e'},
        {"shaping",      no_argument,       0, 's'},
        {"gain1",        required_argument, 0, '1'},
        {"gain2",        required_argument, 0, '2'},
        {"gain3",        required_argument, 0, '3'},
        {"gain4",        required_argument, 0, '4'},
        {"tr_ch",        required_argument, 0, 't'},
        {"tr_level",     required_argument, 0, 'l'},
        {"version",      no_argument,       0, 'v'},
        {"help",         no_argument,       0, 'h'},
        {"hex",          no_argument,       0, 'x'},
        {"volt",         no_argument,       0, 'o'},
        {"calib",        no_argument,       0, 'c'},
        {"hk",           no_argument,       0, 'k'},
        {0, 0, 0, 0}
};

static constexpr char g_format_125_14_4ch[2048] =
        "\n"
        "Usage: %s [OPTION]... SIZE <DEC>\n"
        "\n"
        "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
        "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
        "  --gain1=g       -1 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
        "  --gain2=g       -2 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
        "  --gain3=g       -3 g  Use Channel 3 gain setting g [lv, hv] (default: lv).\n"
        "  --gain4=g       -4 g  Use Channel 4 gain setting g [lv, hv] (default: lv).\n"
        "  --tr_ch=c       -t c  Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N, 3P, 3N, 4P, 4N, EP (external channel), EN (external channel)].\n"
        "                        P - positive edge, N -negative edge. By default trigger no set\n"
        "  --tr_level=c    -l c  Set trigger level (default: 0).\n"
        "  --version       -v    Print version info.\n"
        "  --help          -h    Print this message.\n"
        "  --hex           -x    Print value in hex.\n"
        "  --volt          -o    Print value in volt.\n"
        "  --calib         -c    Disable calibration parameters\n"
        "  --hk            -k    Reset houskeeping (Reset state for GPIO). Default: disabled\n"
        "    SIZE                Number of samples to acquire [0 - %u].\n"
        "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
        "\n";



std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}


int get_float(float *value, const char *str,const char *message,int min_value, int max_value)
{
    try
    {
        if ( str == NULL || *str == '\0' )
            throw std::invalid_argument("null string");
        if (sscanf (str,"%f",value) != 1){
            throw std::invalid_argument("invalid input string");
        }
    }
    catch (...)
    {
        fprintf(stderr, "%s: %s\n",message, str);
        return -1;
    }
    if (*value < min_value){
        fprintf(stderr, "%s: %s\n",message, str);
        return -1;
    }
    if (*value > max_value){
        fprintf(stderr, "%s: %s\n",message, str);
        return -1;
    }
    return 0;
}

/** Print usage information */
auto usage(char const* progName) -> void{

    auto arr = split(std::string(progName),'/');

    std::string name = "";
    if (arr.size() > 0)
        name = arr[arr.size()-1];

    auto n = name.c_str();

    auto *g_format = &g_format_125_14;
    auto model = getModel();
    if (model == RP_125_14_4CH)
        g_format = &g_format_125_14_4ch;
    if (model == RP_250_12)
        g_format = &g_format_250_12;

    fprintf(stderr,"%s Version: %s-%s\n",n,VERSION_STR, REVISION_STR);
    fprintf(stderr, (char*)g_format, n, ADC_BUFFER_SIZE,
            g_dec[0],
            g_dec[1],
            g_dec[2],
            g_dec[3],
            g_dec[4]);
}

auto parse(int argc, char* argv[]) -> Options{
    Options opt;
    if (argc < 2) return opt;
    int option_index = 0;
    int ch = -1;

    opt.error = false;

    auto *optstring = &optstring_125_14;
    auto *long_options = &long_options_125_14;
    auto model = getModel();
    if (model == RP_125_14_4CH){
        optstring = &optstring_125_14_4ch;
        long_options = &long_options_125_14_4ch;
    }
    if (model == RP_250_12){
        optstring = &optstring_250_12;
        long_options = &long_options_250_12;
    }

    while ((ch = getopt_long(argc, argv, (char*)optstring, (option*)long_options, &option_index)) != -1) {
        switch (ch) {

            case '1': {
                if (model == RP_125_14 || model == RP_125_14_4CH){
                    if (strcmp(optarg, "lv") == 0) {
                        opt.attenuator_mode[0] = RP_LOW;
                    } else if (strcmp(optarg, "hv") == 0) {
                        opt.attenuator_mode[0] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                }

                if (model == RP_250_12){
                    if (strcmp(optarg, "1") == 0) {
                        opt.attenuator_mode[0] = RP_LOW;
                    } else if (strcmp(optarg, "20") == 0) {
                        opt.attenuator_mode[0] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                }
                break;
            }

            case '2': {
                if (model == RP_125_14 || model == RP_125_14_4CH){
                    if (strcmp(optarg, "lv") == 0) {
                        opt.attenuator_mode[1] = RP_LOW;
                    } else if (strcmp(optarg, "hv") == 0) {
                        opt.attenuator_mode[1] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                }

                if (model == RP_250_12){
                    if (strcmp(optarg, "1") == 0) {
                        opt.attenuator_mode[1] = RP_LOW;
                    } else if (strcmp(optarg, "20") == 0) {
                        opt.attenuator_mode[1] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                }
                break;
            }


            case '3': {
                if (model == RP_125_14_4CH){
                    if (strcmp(optarg, "lv") == 0) {
                        opt.attenuator_mode[2] = RP_LOW;
                    } else if (strcmp(optarg, "hv") == 0) {
                        opt.attenuator_mode[2] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                }
                break;
            }

            case '4': {
                if (model == RP_125_14_4CH){
                    if (strcmp(optarg, "lv") == 0) {
                        opt.attenuator_mode[3] = RP_LOW;
                    } else if (strcmp(optarg, "hv") == 0) {
                        opt.attenuator_mode[3] = RP_HIGH;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.error = true;
                        return opt;
                    }
                }
                break;
            }

            case 'r': {
                opt.disableReset = true;
                break;
            }

            case 't': {
                if (strcmp(optarg, "1P") == 0) {
                    opt.trigger_mode = RP_TRIG_SRC_CHA_PE;
                    break;
                }

                if (strcmp(optarg, "1N") == 0) {
                    opt.trigger_mode = RP_TRIG_SRC_CHA_NE;
                    break;
                }

                if (strcmp(optarg, "2P") == 0) {
                    opt.trigger_mode = RP_TRIG_SRC_CHB_PE;
                    break;
                }

                if (strcmp(optarg, "2N") == 0) {
                    opt.trigger_mode = RP_TRIG_SRC_CHB_NE;
                    break;
                }

                if (strcmp(optarg, "EP") == 0) {
                    opt.trigger_mode = RP_TRIG_SRC_EXT_PE;
                    break;
                }

                if (strcmp(optarg, "EN") == 0) {
                    opt.trigger_mode = RP_TRIG_SRC_EXT_NE;
                    break;
                }

                if (model == RP_125_14_4CH){
                    if (strcmp(optarg, "3P") == 0) {
                        opt.trigger_mode = RP_TRIG_SRC_CHC_PE;
                        break;
                    }

                    if (strcmp(optarg, "3N") == 0) {
                        opt.trigger_mode = RP_TRIG_SRC_CHC_NE;
                        break;
                    }

                    if (strcmp(optarg, "4P") == 0) {
                        opt.trigger_mode = RP_TRIG_SRC_CHD_PE;
                        break;
                    }

                    if (strcmp(optarg, "4N") == 0) {
                        opt.trigger_mode = RP_TRIG_SRC_CHD_NE;
                        break;
                    }
                }

                fprintf(stderr, "Error key --get: %s\n", optarg);
                opt.error = true;
                return opt;
            }


            case 'l': {
                float trig_level = 0;
                if (get_float(&trig_level, optarg, "Error get trigger level",-10, 10) != 0) {
                    opt.error = true;
                    return opt;
                }
                opt.trigger_level = trig_level;
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

            case 'a': {
                opt.enableAXI = true;
                opt.reset_hk = true;
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

        if (opt.decimation <= 16){
            auto findDec = false;
            for (int idx = 0; idx < DEC_MAX; idx++) {
                if (opt.decimation == g_dec[idx]) {
                    findDec = true;
                }
            }

            if(!findDec){
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

uint8_t getChannels(){
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

models_t getModel(){
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get board model\n");
    }

    switch (c)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
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
            return RP_250_12;
        default:
            fprintf(stderr,"[Error] Can't get board model\n");
            exit(-1);
    }
    return RP_125_14;
}