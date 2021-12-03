#include <iostream>
#include <getopt.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <chrono>

#include "options.h"

static struct option long_options_broadcast[] = {
        /* These options set a flag. */
        {"detect",       no_argument,       0, 'd'},
        {"port",         required_argument, 0, 'p'},
        {"timeout",      required_argument, 0, 't'},
        {0, 0, 0, 0}
};

static constexpr char optstring_broadcast[] = "dp:t:";

static struct option long_options_config[] = {
        /* These options set a flag. */
        {"config",       no_argument,       0, 'c'},
        {"hosts",        required_argument, 0, 'h'},
        {"port",         required_argument, 0, 'p'},
        {"get",          required_argument, 0, 'g'},
        {"set",          required_argument, 0, 's'},
        {"config_file",  required_argument, 0, 'f'},
        {"verbose",      no_argument,       0, 'v'},
        {0, 0, 0, 0}
};

static constexpr char optstring_config[] = "ch:p:g:s:f:v";

static struct option long_options_remote[] = {
        /* These options set a flag. */
        {"remote",       no_argument,       0, 'r'},
        {"hosts",        required_argument, 0, 'h'},
        {"port",         required_argument, 0, 'p'},
        {"mode",         required_argument, 0, 'm'},
        {"timeout",      required_argument, 0, 't'},
        {"verbose",      no_argument,       0, 'v'},
        {0, 0, 0, 0}
};

static constexpr char optstring_remote[] = "rh:p:m:t:v";

static struct option long_options_streaming[] = {
        /* These options set a flag. */
        {"streaming",    no_argument,       0, 's'},
        {"hosts",        required_argument, 0, 'h'},
        {"port",         required_argument, 0, 'p'},
        {"config_port",  required_argument, 0, 'c'},
        {"format",       required_argument, 0, 'f'},
        {"dir",          required_argument, 0, 'd'},
        {"limit",        required_argument, 0, 'l'},
        {"mode",         required_argument, 0, 'm'},
        {"timeout",      required_argument, 0, 't'},
        {"verbose",      no_argument,       0, 'v'},
        {0, 0, 0, 0}
};

static constexpr char optstring_streaming[] = "sh:p:c:f:d:l:m:t:v";

static struct option long_options_dac_streaming[] = {
        /* These options set a flag. */
        {"out_streaming", no_argument,       0, 'o'},
        {"hosts",         required_argument, 0, 'h'},
        {"port",          required_argument, 0, 'p'},
        {"config_port",   required_argument, 0, 'c'},
        {"format",        required_argument, 0, 'f'},
        {"data",          required_argument, 0, 'd'},
        {"repeat",        required_argument, 0, 'r'},
        {"memory",        required_argument, 0, 'm'},
        {"verbose",       no_argument,       0, 'v'},
        {0, 0, 0, 0}
};

static constexpr char optstring_dac_streaming[] = "oh:p:c:f:d:r:m:v";

auto getTS(std::string suffix) -> std::string{

    using namespace std;
    using namespace std::chrono;
    system_clock::time_point timeNow = system_clock::now();
    auto ttime_t = system_clock::to_time_t(timeNow);
    auto tp_sec = system_clock::from_time_t(ttime_t);
    milliseconds ms = duration_cast<milliseconds>(timeNow - tp_sec);

    std::tm * ttm = localtime(&ttime_t);

    char date_time_format[] = "%Y.%m.%d-%H.%M.%S";

    char time_str[] = "yyyy.mm.dd.HH-MM.SS.fff";

    strftime(time_str, strlen(time_str), date_time_format, ttm);

    string result(time_str);
    result.append(".");
    result.append(to_string(ms.count()));
    result.append(suffix);
    return result;
}

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

int get_int(int *value, const char *str,const char *message,int min_value, int max_value)
{
    try
    {
        if ( str == NULL || *str == '\0' )
            throw std::invalid_argument("null string");
        auto checkstr = str;
        while(*checkstr)
        {
            if ( *checkstr < '0' || *checkstr > '9' )
                throw std::invalid_argument("invalid input string");
            ++checkstr;
        }
        *value = std::stoi(str);
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

int get_memory(int64_t *value, const char *str,const char *message)
{
    try
    {
        if ( str == NULL || *str == '\0' )
            throw std::invalid_argument("null string");
        auto strLen = strlen(str);
        if ( strLen >= 255 )
            throw std::invalid_argument("invalid input string");
        char checkstr[255];
        strncpy(checkstr,str,strLen);
        bool subSuffix = false;
        int multipyMode = 1;
        if (str[strLen-1] == 'M'){
            subSuffix = true;
            multipyMode = 1024 * 1024;
        } else if (str[strLen-1] == 'k'){
            subSuffix = true;
            multipyMode = 1024;
        }
        if (subSuffix){
            checkstr[strlen(checkstr) - 1] = 0;
        }
        int idx = 0;
        while(checkstr[idx])
        {
            if ( checkstr[idx] < '0' || checkstr[idx] > '9' ){
                throw std::invalid_argument("invalid input string");
            }
            ++idx;
        }
        *value = std::stoi(str);
        *value *= multipyMode;
    }
    catch (...)
    {
        fprintf(stderr, "%s: %s\n",message, str);
        return -1;
    }
    return 0;
}

auto check_ip( const std::string &value ) -> bool {
    int octet[4], l;
    int r = sscanf (value.c_str(), "%d.%d.%d.%d%n", octet, octet + 1, octet + 2, octet + 3, &l);
    if (r != 4 || l != (int)value.length ()) {
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (octet[i] < 0 || octet[i] >= 256) {
            return false;
        }
    }
    return true;
}

void remove_duplicates(std::vector<std::string>& vec)
{
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}


/** Print usage information */
auto ClientOpt::usage(char const* progName) -> void{
#ifdef _WIN32
    auto arr = split(std::string(progName),'\\');
#else
    auto arr = split(std::string(progName),'/');
#endif

    std::string name = "";
    if (arr.size() > 0)
        name = arr[arr.size()-1];
    const char *format =
            "\n"
            "Usage: %s\n"
            "\n"
            "Detect Mode:\n"
            "\tThis mode allows you to determine the IP addresses that are in the network in streaming mode. By default, the search takes 5 seconds.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -d [-p PORT] [-t SEC]\n"
            "\t\t%s --detect [--port=PORT] [--timeout=SEC]\n"
            "\n"
            "\t\t--detect               -d           Enable broadcast search.\n"
            "\t\t--port=PORT            -p PORT      Port for broadcast (Default: 8902).\n"
            "\t\t--timeout=SEC          -t SEC       Timeout(Default: 5 sec).\n"
            "\n"
            "Configuration Mode:\n"
            "\tThis mode allows you to get or set the configuration on the boards.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -c -h IPs [-p PORT] -g V|VV|F [-v]\n"
            "\t\t%s -c -h IPs [-p PORT] -s M|F [-f FILE] [-v]\n"
            "\t\t%s --config --hosts=IPs [--port=PORT] --get=V|VV|F [--verbose]\n"
            "\t\t%s --config --hosts=IPs [--port=PORT] --set=M|F [--config_file=FILE] [--verbose]\n"
            "\n"
            "\t\t--config               -c           Enable config mode.\n"
            "\t\t--hosts=IP,...         -h IP,...    You can specify one or more board IP addresses through a separator - ','\n"
            "\t\t                                    Example: --hosts=127.0.0.1 or --hosts=127.0.0.1,127.0.0.2\n"
            "\t\t                                              -p 127.0.0.1     or  -p 127.0.0.1,127.0.0.2,127.0.0.3\n"
            "\t\t--port=PORT            -p PORT      Port for configuration server (Default: 8901).\n"
            "\t\t--get=V|V1|VV|F        -g V|V1|VV|F Requests configurations from all boards.\n"
            "\t\t                                    Keys: V  = Displays on the screen in json format.\n"
            "\t\t                                          V1 = Displays on the screen in json format (only data).\n"
            "\t\t                                          VV = Displays on the screen in a format with decoding values.\n"
            "\t\t                                          F  = Saves to a config files.\n"
            "\t\t--set=M|F              -s M|F       Sets configurations for all boards.\n"
            "\t\t                                    Keys: M  = Sets values only to memory without saving to file.\n"
            "\t\t                                          F  = Sets configuration and saves to file on remote boards.\n"
            "\t\t--config_file=FILE     -f FILE      Configuration file for setting values on boards (Default: config.json).\n"
            "\t\t--verbose              -v           Displays service information.\n"
            "\n"
            "Remote control Mode:\n"
            "\tThis mode allows you to control streaming as a client.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -r -h IPs [-p PORT] -m start|stop|start_stop|start_dac|stop_dac|start_stop_dac [-t MSEC] [-v]\n"
            "\t\t%s --remote --hosts=IPs [--port=PORT] --mode=start|stop|start_stop|start_dac|stop_dac|start_stop_dac [--timeout=MSEC] [--verbose]\n"
            "\n"
            "\t\t--remote               -r           Enable remote control mode.\n"
            "\t\t--hosts=IP,...         -h IP,...    You can specify one or more board IP addresses through a separator - ','\n"
            "\t\t                                    Example: --hosts=127.0.0.1 or --hosts=127.0.0.1,127.0.0.2\n"
            "\t\t                                              -p 127.0.0.1     or  -p 127.0.0.1,127.0.0.2,127.0.0.3\n"
            "\t\t--port=PORT            -p PORT      Port for configuration server (Default: 8901).\n"
            "\t\t--mode=MODE            -m MODE      Commands for managing servers.\n"
            "\t\t                                     Keys: start = Starts the server.\n"
            "\t\t                                           stop = Stop the server.\n"
            "\t\t                                           start_stop = Sends a start command at the end of the timeout sends a stop command.\n"
            "\t\t                                           start_dac = Starts the DAC server.\n"
            "\t\t                                           stop_dac = Stop the DAC server.\n"
            "\t\t                                           start_stop_dac = Sends a start command at the end of the timeout sends a stop command for DAC mode.\n"
            "\t\t--timeout=MSEC         -t MSEC      Timeout (Default: 1000 ms). Used only in conjunction with the start_stop command.\n"
            "\t\t--verbose              -v           Displays service information.\n"
            "\n"
            "Streaming Mode:\n"
            "\tThis mode allows you to control streaming as a client, and also captures data in network streaming mode.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -s -h IPs [-p PORT] [-c PORT] -f tdms|wav|csv [-d NAME] [-m raw|volt] [-l SAMPLES] [-t MSEC] [-v]\n"
            "\t\t%s --streaming --hosts=IPs [--port=PORT] [--config_port=PORT] --format=tdms|wav|csv [--dir=NAME] [--limit=SAMPLES] [--mode=raw|volt] [--timeout=MSEC] [--verbose]\n"
            "\n"
            "\t\t--streaming            -s           Enable streaming mode.\n"
            "\t\t--hosts=IP,...         -h IP,...    You can specify one or more board IP addresses through a separator - ','\n"
            "\t\t                                    Example: --hosts=127.0.0.1 or --hosts=127.0.0.1,127.0.0.2\n"
            "\t\t                                              -p 127.0.0.1     or  -p 127.0.0.1,127.0.0.2,127.0.0.3\n"
            "\t\t--port=PORT            -p PORT      Port for streaming server (Default: 8900).\n"
            "\t\t--config_port=PORT     -c PORT      Port for configuration server (Default: 8901).\n"
            "\t\t--format=FORMAT        -f FORMAT    The format in which the data will be saved.\n"
            "\t\t                                    Keys: tdsm = NI TDMS File Format.\n"
            "\t\t                                          wav = Waveform Audio File Format.\n"
            "\t\t                                          csv = Text file that uses a comma to separate values.\n"
            "\t\t--dir=NAME             -d NAME      Path to the directory where to save files.\n"
            "\t\t--limit=SAMPLES        -l SAMPLES   Sample limit [1-%d] (no limit by default).\n"
            "\t\t--mode=MODE            -m MODE      Convert values in volts (store as ADC raw data by default).\n"
            "\t\t                                    Keys: raw = 8/16 Bit binary raw format.\n"
            "\t\t                                          volt = Converts binary integer format to floating point format.\n"
            "\t\t                                                 Measured in volts. In wav format, it is limited from -1 to 1.\n"
            "\t\t--timeout=MSEC         -t MSEC      Stops recording after a specified amount of time.\n"
            "\t\t--verbose              -v           Displays service information.\n"
            "\n"
            "DAC streaming Mode:\n"
            "\tThis mode allows you to generate output data using a signal from a file.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -o -h IPs [-p PORT] [-c PORT] -f tdms|wav -d FILE_NAME [-r inf|COUNT] [-m SIZE] [-v]\n"
            "\t\t%s --out_streaming --hosts=IPs [--port=PORT] [--config_port=PORT] --format=tdms|wav --data=FILE_NAME [--repeat=inf|COUNT] [--memory SIZE] [--verbose]\n"
            "\t\t%s -oc CONFIG_FILE\n"
            "\t\t%s --out_streaming_conf CONFIG_FILE\n"
            "\n"
            "\t\t--out_streaming        -o           Enable dac streaming mode.\n"
            "\t\t--out_streaming_conf   -oc          Enable dac streaming mode.\n"
            "\t\t--hosts=IP,...         -h IP,...    You can specify one or more board IP addresses through a separator - ','\n"
            "\t\t                                    Example: --hosts=127.0.0.1 or --hosts=127.0.0.1,127.0.0.2\n"
            "\t\t                                              -p 127.0.0.1     or  -p 127.0.0.1,127.0.0.2,127.0.0.3\n"
            "\t\t--port=PORT            -p PORT      Port for dac streaming server (Default: 8903).\n"
            "\t\t--config_port=PORT     -c PORT      Port for configuration server (Default: 8901).\n"
            "\t\t--format=FORMAT        -f FORMAT    The format in which the data will be used.\n"
            "\t\t                                    Keys: tdsm = NI TDMS File Format.\n"
            "\t\t                                          wav = Waveform Audio File Format.\n"
            "\t\t--data=FILE_NAME       -d FILE_NAME Path to the file for streaming.\n"
            "\t\t--memory=SIZE          -m SIZE      Use RAM cache.\n"
            "\t\t                                        Example: --mmemory 1048576 or --memory 1M or --memory 1024k\n"
            "\t\t                                                 -m 1048576 or -m 1M or -m 1024k\n"
            "\t\t                                        Default: 1M bytes\n"
            "\t\t--repeat=inf|COUNT     -r inf|COUNT The number of times the file has been resent.\n"
            "\t\t                                    Keys: inf is an infinite number of times.\n"
            "\t\t                                       COUNT - value from [1 ... 1000000]\n"
            "\t\t--verbose              -v           Displays service information.\n"
            "\n";
    auto n = name.c_str();
    fprintf( stderr, format, n,n,n,n,n,n,n,n,n,n,n,0x7FFFFFFF,n,n,n,n);
}

auto ClientOpt::parse(int argc, char* argv[]) -> ClientOpt::Options{
    Options opt;
    opt.mode = Mode::ERROR_MODE;
    if (argc < 2) return opt;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int ch = -1;
    if ((strcmp(argv[1],"-d") == 0) || (strcmp(argv[1],"--detect") == 0)) {
        while ((ch = getopt_long(argc, argv, optstring_broadcast, long_options_broadcast, &option_index)) != -1) {
            switch (ch) {

                case 'd':
                    opt.mode = Mode::SEARCH;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number",1, 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.ports.broadcast_port = optarg;
                    break;
                }

                case 't': {
                    int t_out = 0;
                    if (get_int(&t_out, optarg, "Error get timout",0, 100000) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = t_out;
                    break;
                }
                default: {
                    if (opt.mode == Mode::SEARCH) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        if (opt.mode != Mode::ERROR_MODE){
            return opt;
        }
    }

    option_index = 0;
    ch = -1;
    if ((strcmp(argv[1],"-c") == 0) || (strcmp(argv[1],"--config") == 0)) {
        while ((ch = getopt_long(argc, argv, optstring_config, long_options_config, &option_index)) != -1) {
            switch (ch) {

                case 'c':
                    opt.mode = Mode::CONFIG;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number",1, 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.ports.config_port = optarg;
                    break;
                }

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto &s:opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "Error parse ip address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "Error parse ip address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'g': {
                    if (strcmp(optarg, "V") == 0) {
                        opt.conf_get = ConfGet::VERBOUS_JSON;
                    } else if (strcmp(optarg, "V1") == 0) {
                        opt.conf_get = ConfGet::VERBOUS_JSON_DATA;
                    } else if (strcmp(optarg, "VV") == 0) {
                        opt.conf_get = ConfGet::VERBOUS;
                    } else if (strcmp(optarg, "F") == 0) {
                        opt.conf_get = ConfGet::FILE;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 's': {
                    if (strcmp(optarg, "M") == 0) {
                        opt.conf_set = ConfSet::MEMORY;
                    } else if (strcmp(optarg, "F") == 0) {
                        opt.conf_set = ConfSet::FILE;
                    } else {
                        fprintf(stderr, "Error key --set: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 't': {
                    int t_out = 0;
                    if (get_int(&t_out, optarg, "Error get timeout",0, 100000) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = t_out;
                    break;
                }

                case 'f': {
                    if (strcmp(optarg, "") != 0) {
                        opt.conf_file = optarg;
                    } else {
                        fprintf(stderr, "Error key --file: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                default: {
                    if (opt.mode == Mode::CONFIG) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        if (opt.mode != Mode::ERROR_MODE){
            if (opt.mode == Mode::CONFIG && (opt.hosts.size() == 0 || (opt.conf_get == ConfGet::NONE && opt.conf_set == ConfSet::NONE))){
                fprintf(stderr,"[ERROR] Missing required key in configuration mode\n");
                exit( EXIT_FAILURE );
            }
            return opt;
        }
    }

    option_index = 0;
    ch = -1;
    if ((strcmp(argv[1],"-r") == 0) || (strcmp(argv[1],"--remote") == 0)) {
        while ((ch = getopt_long(argc, argv, optstring_remote, long_options_remote, &option_index)) != -1) {
            switch (ch) {

                case 'r':
                    opt.mode = Mode::REMOTE;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number",1, 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.ports.config_port = optarg;
                    break;
                }

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto &s:opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "Error parse ip address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "Error parse ip address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'm': {
                    if (strcmp(optarg, "start") == 0) {
                        opt.remote_mode = RemoteMode::START;
                    } else if (strcmp(optarg, "stop") == 0) {
                        opt.remote_mode = RemoteMode::STOP;
                    } else if (strcmp(optarg, "start_stop") == 0) {
                        opt.remote_mode = RemoteMode::START_STOP;
                    } else if (strcmp(optarg, "start_dac") == 0) {
                        opt.remote_mode = RemoteMode::START_DAC;
                    } else if (strcmp(optarg, "stop_dac") == 0) {
                        opt.remote_mode = RemoteMode::STOP_DAC;
                    } else if (strcmp(optarg, "start_stop_dac") == 0) {
                        opt.remote_mode = RemoteMode::START_STOP_DAC;
                    } else {
                        fprintf(stderr, "Error key --mode: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 't': {
                    int t_out = 0;
                    if (get_int(&t_out, optarg, "Error get timeout",0, 0xFFFFFFF) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = t_out;
                    break;
                }

                default: {
                    if (opt.mode == Mode::CONFIG) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        if (opt.mode != Mode::ERROR_MODE){
            if (opt.mode == Mode::REMOTE && (opt.remote_mode ==RemoteMode::NONE || opt.hosts.size() == 0)){
                fprintf(stderr,"[ERROR] Missing required key in remote mode\n");
                exit( EXIT_FAILURE );
            }
            return opt;
        }
    }

    option_index = 0;
    ch = -1;
    if ((strcmp(argv[1],"-s") == 0) || (strcmp(argv[1],"--streaming") == 0)) {
        opt.timeout = -1;
        while ((ch = getopt_long(argc, argv, optstring_streaming, long_options_streaming, &option_index)) != -1) {
            switch (ch) {

                case 's':
                    opt.mode = Mode::STREAMING;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number", 1,65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.ports.streaming_port = optarg;
                    break;
                }

                case 'c': {
                    int conf_port = 0;
                    if (get_int(&conf_port, optarg, "Error get port number for configuration server",1, 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.ports.config_port = optarg;
                    break;
                }

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto &s:opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "Error parse ip address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "Error parse ip address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'f': {
                    if (strcmp(optarg, "tdms") == 0) {
                        opt.streamign_type = StreamingType::TDMS;
                    } else if (strcmp(optarg, "wav") == 0) {
                        opt.streamign_type = StreamingType::WAV;
                    } else if (strcmp(optarg, "csv") == 0) {
                        opt.streamign_type = StreamingType::CSV;
                    } else {
                        fprintf(stderr, "Error key --format: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'm': {
                    if (strcmp(optarg, "raw") == 0) {
                        opt.save_type = SaveType::RAW;
                    } else if (strcmp(optarg, "volt") == 0) {
                        opt.save_type = SaveType::VOL;
                    } else {
                        fprintf(stderr, "Error key --mode: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }
                case 'l': {
                    int samp = 0;
                    if (get_int(&samp, optarg, "Error get samples limit",1, 0x7FFFFFFF) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.samples = samp;
                    break;
                }

                case 't': {
                    int t_out = 0;
                    if (get_int(&t_out, optarg, "Error get timeout", 0,0xFFFFFFF) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = t_out;
                    break;
                }

                case 'd': {
                    if (strcmp(optarg, "") != 0) {
                        opt.save_dir = optarg;
                    } else {
                        fprintf(stderr, "Error key --dir: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                default: {
                    if (opt.mode == Mode::CONFIG) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        if (opt.mode != Mode::ERROR_MODE){
            if (opt.mode == Mode::STREAMING && (opt.streamign_type == StreamingType::NONE || opt.hosts.size() == 0)){
                fprintf(stderr,"[ERROR] Missing required key in streaming mode\n");
                exit( EXIT_FAILURE );
            }
            return opt;
        }
    }

    option_index = 0;
    ch = -1;
    if ((strcmp(argv[1],"-o") == 0) || (strcmp(argv[1],"--out_streaming") == 0)) {
        opt.timeout = -1;
        while ((ch = getopt_long(argc, argv, optstring_dac_streaming, long_options_dac_streaming, &option_index)) != -1) {
            switch (ch) {

                case 'o':
                    opt.mode = Mode::STREAMING_DAC;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number", 1,65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.ports.dac_streaming_port = optarg;
                    break;
                }

                case 'm': {
                    int64_t memory = 0;
                    if (get_memory(&memory, optarg, "Error get memory size") != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.dac_memory = 0;
                    break;
                }

                case 'c': {
                    int conf_port = 0;
                    if (get_int(&conf_port, optarg, "Error get port number for configuration server",1, 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.ports.config_port = optarg;
                    break;
                }

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto &s:opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "Error parse ip address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "Error parse ip address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'f': {
                    if (strcmp(optarg, "tdms") == 0) {
                        opt.streamign_type = StreamingType::TDMS;
                    } else if (strcmp(optarg, "wav") == 0) {
                        opt.streamign_type = StreamingType::WAV;
                    } else {
                        fprintf(stderr, "Error key --format: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'r': {
                    int repeat = 0;
                    if (strcmp(optarg, "inf") == 0) {
                        opt.dac_repeat = (int)RepeatDAC::INF;
                    }else if (get_int(&repeat, optarg, "Error get repeat count",1, 1000000) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }else{
                        opt.dac_repeat = repeat;
                    }
                    
                    break;
                }

                case 'd': {
                    if (strcmp(optarg, "") != 0) {
                        opt.dac_file = optarg;
                    } else {
                        fprintf(stderr, "Error key --data: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                default: {
                    if (opt.mode == Mode::CONFIG) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        if (opt.mode != Mode::ERROR_MODE){
            if (opt.mode == Mode::STREAMING_DAC && (opt.streamign_type == StreamingType::NONE || opt.hosts.size() == 0)){
                fprintf(stderr,"[ERROR] Missing required key in dac streaming mode\n");
                exit( EXIT_FAILURE );
            }
            return opt;
        }
    }

    option_index = 0;
    ch = -1;
    if ((strcmp(argv[1],"-oc") == 0) || (strcmp(argv[1],"--out_streaming_conf") == 0)) {
        opt.mode = Mode::STREAMING_DAC_CONF;

        if (argc > 2) {
            opt.streaming_conf_file = argv[2];
        }

        if (opt.mode != Mode::ERROR_MODE){
            if (opt.mode == Mode::STREAMING_DAC_CONF && opt.streaming_conf_file == ""){
                fprintf(stderr,"[ERROR] Missing required key in dac streaming mode\n");
                exit( EXIT_FAILURE );
            }
            return opt;
        }
    }

    return opt;
}

auto time_point_to_string(std::chrono::system_clock::time_point &tp) -> std::string
{
    using namespace std;
    using namespace std::chrono;

    auto ttime_t = system_clock::to_time_t(tp);
    auto tp_sec = system_clock::from_time_t(ttime_t);
    milliseconds ms = duration_cast<milliseconds>(tp - tp_sec);

    std::tm * ttm = localtime(&ttime_t);

    char date_time_format[] = "%Y.%m.%d-%H.%M.%S";

    char time_str[] = "yyyy.mm.dd.HH-MM.SS.fff";

    strftime(time_str, strlen(time_str), date_time_format, ttm);

    string result(time_str);
    result.append(".");
    result.append(to_string(ms.count()));

    return result;
}
