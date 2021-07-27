#include <iostream>
#include <getopt.h>
#include <vector>

#include "options.h"

static struct option long_options_broadcast[] = {
        /* These options set a flag. */
        {"search",       no_argument,       0, 's'},
        {"port",         required_argument, 0, 'p'},
        {"timeout",      required_argument, 0, 't'},
        {0, 0, 0, 0}
};

static constexpr char optstring_broadcast[] = "sp:t:";

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

int get_int(int *value, const char *str,const char *message,int max_value)
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
    if (*value > max_value){
        fprintf(stderr, "%s: %s\n",message, str);
        return -1;
    }
    return 0;
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
            "\tBroadcast mode:\n"
            "\t\tThis mode allows you to determine the IP addresses that are in the network in streaming mode. By default, the search takes 5 seconds.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -s [-p PORT] [-t SEC]\n"
            "\t\t%s --search [--port=PORT] [--timeout=SEC]\n"
            "\n"
            "\t\t--search       -s        Enable broadcast search.\n"
            "\t\t--port=PORT    -p PORT   Port for broadcast (Default: 8902).\n"
            "\t\t--timeout=SEC  -t SEC    Timeout(Default: 5 sec).\n"
            "\n"
            "\n"
            "\n"
            "\n"

            "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
            "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
            "  --gain1=g       -1 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
            "  --gain2=g       -2 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
            "  --tr_ch=c       -t c  Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N].\n"
            "                        P - positive edge, N -negative edge. By default trigger no set\n"
            "  --tr_level=c    -l c  Set trigger level (default: 0).\n"
            "  --version       -v    Print version info.\n"
            "  --help          -h    Print this message.\n"
            "  --hex           -x    Print value in hex.\n"
            "  --volt          -o    Print value in volt.\n"
            "  --calib         -c    Disable calibration parameters\n"
            "\n";

    fprintf( stderr, format, name.c_str(), name.c_str(),name.c_str());
}

auto ClientOpt::parse(int argc, char* argv[]) -> ClientOpt::Optons{
    Optons opt;
    opt.mode = Mode::ERROR;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int ch = -1;
    while ( (ch = getopt_long( argc, argv, optstring_broadcast, long_options_broadcast, &option_index )) != -1 ) {
        switch ( ch ) {

            case 's':
                opt.mode = Mode::SEARCH;
                break;

            case 'p':
            {
                int port = 0;
                if (get_int(&port, optarg,"Error get port number",65535) != 0) {
                    opt.mode = Mode::ERROR_PARAM;
                    return opt;
                }
                opt.port = optarg;
                break;
            }

            case 't':
            {
                int t_out = 0;
                if (get_int(&t_out, optarg,"Error get timout",100000) != 0) {
                    opt.mode = Mode::ERROR_PARAM;
                    return opt;
                }
                opt.timeout = t_out;
                break;
            }
            default:
            {
                if (opt.mode == Mode::SEARCH){
                    fprintf(stderr,"Unknown parameter\n");
                    exit( EXIT_FAILURE );
                }
            }
        }
    }
}
