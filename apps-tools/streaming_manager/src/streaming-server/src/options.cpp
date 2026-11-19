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
#include "asio.hpp"

static struct option long_options[] = {
    /* These options set a flag. */
    {"background", no_argument, 0, 'b'},
    {"file", required_argument, 0, 'f'},
    {"listen", required_argument, 0, 'l'},
    {"verbose", no_argument, 0, 'v'},
    {"web", required_argument, 0, 'w'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {0, 0, 0, 0}};

static constexpr char optstring[] = "bf:hl:vVw:";

#define SERVER_XSTR(s) SERVER_STR(s)
#define SERVER_STR(s) #s
#ifdef VERSION
#define SERVER_VERSION SERVER_XSTR(VERSION) "-" SERVER_XSTR(REVISION)
#else
#define SERVER_VERSION "unknown"
#endif

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
        "\t%s [-b] [-f PATH] [-l ADDRESS] [-w PORT] [-v] [-V]\n"
        "\t%s [--background] [--file=PATH] [--listen=ADDRESS] [--web=PORT] [--verbose] [--version]\n"
        "\n"
        "\t--background          -b        Run service in background.\n"
        "\t--file=PATH           -f FILE   Path to configuration file.\n"
        "\t                                By default uses the config file /root/.config/redpitaya/apps/streaming/streaming_config.json.\n"
        "\t--listen=ADDRESS      -l ADDRESS The address of the board the config and data ports listen on,\n"
        "\t                                for example 127.0.0.1 for the clients of the board only.\n"
        "\t                                By default every interface.\n"
        "\t--web=PORT            -w PORT   The port of the web page on 127.0.0.1 (nginx proxies the page to it);\n"
        "\t                                0 - no web page. By default 18906 on the board.\n"
        "\t--verbose             -v        Displays information.\n"
        "\t--version             -V        The version of the server.\n"
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

            case 'l': {
                // an address of this board: a wrong one would leave the ports closed
                asio::error_code ec;
                asio::ip::make_address(optarg, ec);
                if (ec) {
                    printWithLog(LOG_ERR, stderr, "[ERROR] Wrong value of --listen: %s (an IP address of the board, e.g. 127.0.0.1)\n", optarg);
                    exit(EXIT_FAILURE);
                }
                opt.listen = optarg;
                break;
            }

            case 'V':
                aprintf(stdout, "streaming-server %s\n", SERVER_VERSION);
                exit(EXIT_SUCCESS);

            case 'w': {
                char* end = nullptr;
                const long port = strtol(optarg, &end, 10);
                if (!end || *end != '\0' || port < 0 || port > 65535) {
                    printWithLog(LOG_ERR, stderr, "[ERROR] Wrong value of --web: %s (a port, 0 - no web page)\n", optarg);
                    exit(EXIT_FAILURE);
                }
#ifndef STREAMING_WEB
                if (port != 0) {
                    printWithLog(LOG_ERR, stderr, "[ERROR] --web: this build has no web page\n");
                    exit(EXIT_FAILURE);
                }
#endif
                opt.webPort = (uint16_t)port;
                break;
            }

            default: {
                printWithLog(LOG_ERR, stderr, "[ERROR] Unknown parameter (--help lists them)\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return opt;
}
