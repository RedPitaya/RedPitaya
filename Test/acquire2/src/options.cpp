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


#if defined Z20_250_12
    static constexpr char optstring[] = "esx1:2:d:vht:l:orc";
    static struct option long_options[] = {
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
            {0, 0, 0, 0}
    };

    static constexpr char g_format[] =
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
            "    SIZE                Number of samples to acquire [0 - %u].\n"
            "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
            "\n";
#endif

#if defined Z10 || defined Z20 || defined Z20_125
    static constexpr char optstring[] = "esx1:2:vht:l:oc";
    static struct option long_options[] = {
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
            {0, 0, 0, 0}
    };

    static constexpr char g_format[] =
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
            "    SIZE                Number of samples to acquire [0 - %u].\n"
            "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
            "\n";
#endif

#if defined Z20_125_4CH
    static constexpr char optstring[] = "esx1:2:3:4:vht:l:oc";
    static struct option long_options[] = {
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
            {0, 0, 0, 0}
    };

    static constexpr char g_format[] =
            "\n"
            "Usage: %s [OPTION]... SIZE <DEC>\n"
            "\n"
            "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
            "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
            "  --gain1=g       -1 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
            "  --gain2=g       -2 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
            "  --gain3=g       -3 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
            "  --gain4=g       -4 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
            "  --tr_ch=c       -t c  Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N, 3P, 3N, 4P, 4N, EP (external channel), EN (external channel)].\n"
            "                        P - positive edge, N -negative edge. By default trigger no set\n"
            "  --tr_level=c    -l c  Set trigger level (default: 0).\n"
            "  --version       -v    Print version info.\n"
            "  --help          -h    Print this message.\n"
            "  --hex           -x    Print value in hex.\n"
            "  --volt          -o    Print value in volt.\n"
            "  --calib         -c    Disable calibration parameters\n"
            "    SIZE                Number of samples to acquire [0 - %u].\n"
            "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
            "\n";
#endif



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
    
    fprintf(stderr,"%s Version: %s-%s\n",n,VERSION_STR, REVISION_STR);
    fprintf(stderr, g_format, n, ADC_BUFFER_SIZE,
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

    while ((ch = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
        switch (ch) {

#if defined Z10 || defined Z20 || defined Z20_125 || defined Z20_125_4CH
            case '1': {
                if (strcmp(optarg, "lv") == 0) {
                    opt.attenuator_mode[0] = RP_LOW;
                } else if (strcmp(optarg, "hv") == 0) {
                    opt.attenuator_mode[0] = RP_HIGH;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }

            case '2': {
                if (strcmp(optarg, "lv") == 0) {
                    opt.attenuator_mode[1] = RP_LOW;
                } else if (strcmp(optarg, "hv") == 0) {
                    opt.attenuator_mode[1] = RP_HIGH;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }
#endif

#if defined Z20_125_4CH
            case '3': {
                if (strcmp(optarg, "lv") == 0) {
                    opt.attenuator_mode[2] = RP_LOW;
                } else if (strcmp(optarg, "hv") == 0) {
                    opt.attenuator_mode[2] = RP_HIGH;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }

            case '4': {
                if (strcmp(optarg, "lv") == 0) {
                    opt.attenuator_mode[3] = RP_LOW;
                } else if (strcmp(optarg, "hv") == 0) {
                    opt.attenuator_mode[3] = RP_HIGH;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.error = true;
                    return opt;
                }
                break;
            }
#endif

#if defined Z20_250_12
            case '1': {
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

            case '2': {
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

            case 'r': {
                opt.disableReset = true;
                break;
            }
#endif

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

#if defined Z20_125_4CH
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
#endif

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

            case 'o': {
                opt.showInVolt = true;
                break;
            }

            case 'c': {
                opt.disableCalibration = true;
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