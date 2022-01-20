#include <iostream>
#include <getopt.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <chrono>

#include "options.h"


static struct option long_options_config[] = {
        /* These options set a flag. */
        {"host",         required_argument, 0, 'h'},
        {"ports",        required_argument, 0, 'p'},
        {"mode",         required_argument, 0, 'm'},
        {"speed",        required_argument, 0, 's'},
        {"verbose",      no_argument,       0, 'v'},
        {"channels",     required_argument, 0, 'c'},
        {"timeout",      required_argument, 0, 't'},
        
        {0, 0, 0, 0}
};

static constexpr char optstring_config[] = "h:p:m:s:vc:t:";


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

int get_ports(ClientOpt::Ports *ports,const char *str){
    try
    {
        auto s_ports = split(str,',');
        if (s_ports.size() != 3){
            throw std::invalid_argument("Invalid ports value");
        }

        int port = 0;
        
        if (get_int(&port, s_ports[0].c_str(), "Error get port number",1, 65535) != 0) {
            return -1;
        }

        ports->streaming_port = s_ports[0];

        if (get_int(&port, s_ports[1].c_str(), "Error get port number",1, 65535) != 0) {
            return -1;
        }

        ports->config_port = s_ports[1];


        if (get_int(&port, s_ports[2].c_str(), "Error get port number",1, 65535) != 0) {
            return -1;
        }

        ports->dac_streaming_port = s_ports[2];

        return 0;
    }
    catch (...)
    {
        fprintf(stderr, "%s\n", str);
        return -1;
    }
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
            checkstr[strLen - 1] = 0;
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
            "Application for testing the quality of the connection of the streaming mode.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -h IP [-p PORT] [-m DD] [-s DAC_SPEED] [-c ONE|TWO] [-t SEC] [-v]\n"
            "\t\t%s --host=IP [--port=PORT] [--mode=DD] [--speed=MAX_SPEED] [--channels ONE|TWO] [--timeout=SEC] [--verbose]\n"
            "\n"
            "\t\t--host=IP              -h IP           Board address for testing\n"
            "\t\t                                       Example: --hosts=127.0.0.1\n"
            "\t\t                                                 -h 127.0.0.1\n"
            "\t\t--ports=PORTS_LIST     -p PORTS_LIST   Ports for server (Default: 8900,8901,8903).\n"
            "\t\t                                       Ports are separated by commas: PORT1,PORT2,PORT3.\n"
            "\t\t                                                 PORT1 = 8900 (Port for ADC streaming server)\n"
            "\t\t                                                 PORT2 = 8901 (Port for configuration server)\n"
            "\t\t                                                 PORT3 = 8903 (Port for DAC streaming server)\n"
            "\t\t--mode=DD              -m DD           Runs the test in the specified mode.\n"
            "\t\t                                       Keys: DD = Bidirectional test. Sends template data to DAC streaming and returns it back.\n"
            "\t\t--speed=SPEED          -s SPEED        Sets the rate at which data is written to the DAC. Limited by the maximum board speed.\n"
            "\t\t--channels=ONE|TWO     -c ONE|TWO      Number of channels used in testing (Default TWO).\n"
            "\t\t--timeout=SEC          -t SEC          Timeout (Default: 10 sec).\n"
            "\t\t--verbose              -v              Displays service information.\n"
            "\n";
    auto n = name.c_str();
    fprintf( stderr, format, n,n,n);
}

auto ClientOpt::parse(int argc, char* argv[]) -> ClientOpt::Options{
    Options opt;
    opt.state = State::ERROR_PARAM;
    if (argc < 2) return opt;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int ch = -1;
    
    opt.state = State::TEST;

    while ((ch = getopt_long(argc, argv, optstring_config, long_options_config, &option_index)) != -1) {
        switch (ch) {

            case 'v':
                opt.verbous = true;
                break;

            case 'p': {
                int port = 0;
                if (get_ports(&opt.ports, optarg) != 0) {
                    opt.state = State::ERROR_PARAM;
                    return opt;
                }
                break;
            }

            case 'h': {
                if (!check_ip(optarg)) {
                    fprintf(stderr, "Error parse ip address: %s\n", optarg);
                    opt.state = State::ERROR_PARAM;
                    return opt;
                }
                opt.host = optarg;
                break;
            }

            case 'm': {
                if (strcmp(optarg, "DD") == 0) {
                    opt.mode = Mode::DD;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.state = State::ERROR_PARAM;
                    return opt;
                }
                break;
            }

            case 's': {
                int s_out = 0;
                if (get_int(&s_out, optarg, "Error get speed",4000, 250e6) != 0) {
                    opt.state = State::ERROR_PARAM;
                    return opt;
                }
                opt.speed = s_out;
                break;
            }

            case 't': {
                int t_out = 0;
                if (get_int(&t_out, optarg, "Error get timeout",0, 100000) != 0) {
                    opt.state = State::ERROR_PARAM;
                    return opt;
                }
                opt.timeout = t_out;
                break;
            }

            case 'c': {
                if (strcmp(optarg, "ONE") == 0) {
                    opt.chs = Channels::ONE;
                } else if (strcmp(optarg, "TWO") == 0) {
                    opt.chs = Channels::TWO;
                } else {
                    fprintf(stderr, "Error key --get: %s\n", optarg);
                    opt.state = State::ERROR_PARAM;
                    return opt;
                }
                break;
            }

            default: {
                if (opt.state == State::TEST) {
                    fprintf(stderr, "[ERROR] Unknown parameter\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    if (opt.state != State::ERROR_PARAM){
        if (opt.state == State::TEST && opt.host == ""){
            fprintf(stderr,"[ERROR] Missing required key\n");
            exit( EXIT_FAILURE );
        }
        return opt;
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
