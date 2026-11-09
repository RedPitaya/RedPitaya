#include <getopt.h>
#include <cstring>
#include <vector>

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
    {"listen", required_argument, 0, 'l'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};

static constexpr char optstring[] = "bf:hl:v";

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
        "\t%s [-b] [-f PATH] [-l ADDRESS] [-v]\n"
        "\t%s [--background] [--file=PATH] [--listen=ADDRESS] [--verbose]\n"
        "\n"
        "\t--background          -b        Run service in background.\n"
        "\t--file=PATH           -f FILE   Path to configuration file.\n"
        "\t                                By default uses the config file /root/.config/redpitaya/apps/streaming/streaming_config.json.\n"
        "\t--listen=ADDRESS      -l ADDRESS The address of the board the config and data ports listen on,\n"
        "\t                                for example 127.0.0.1 for the clients of the board only.\n"
        "\t                                By default every interface.\n"
        "\t--verbose             -v        Displays information.\n"
        "\n"
        "\t Example:\n"
        "\t\t%s -b -f /root/.streaming_config_new.json\n";

    auto n = name.c_str();
    printWithLog(LOG_INFO, stdout, format, n, n, n);
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

            case 'l':
                opt.listen = optarg;
                break;

            default: {
                printWithLog(LOG_ERR, stderr, "[ERROR] Unknown parameter\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return opt;
}
