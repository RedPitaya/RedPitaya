#include <getopt.h>
#include <zip.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <regex>
#include <vector>

#include "options.h"
#include "rp_updater_common.h"

static constexpr char optstring[] = "m:d:vn:li:r";
static struct option long_options[] = {
    {"md5", required_argument, 0, 'm'}, {"download", required_argument, 0, 'd'}, {"install", required_argument, 0, 'i'},     {"verbose", no_argument, 0, 'v'},
    {"list", no_argument, 0, 'l'},      {"list_nb", no_argument, 0, 'r'},        {"download_nb", required_argument, 0, 'n'}, {0, 0, 0, 0}};

static constexpr char g_format[] =
    "\n"
    "Usage: %s -m file,file,...\n"
    "       %s -d URL [-v]\n"
    "       %s -n FILE [-v]\n"
    "       %s -n NUMBER [-v]\n"
    "       %s -i FILE [-v]\n"
    "       %s -i NUMBER [-v]\n"
    "       %s -l\n"
    "       %s -r\n"
    "\n"
    "  --md5=FILES           -m FILES     Calculates md5 for the specified files.\n"
    "  --download=URL        -d URL       Downloads a file to a directory: %s.\n"
    "  --download_nb=FILE    -n FILE      Download ecosystem by file name from NB server.\n"
    "  --download_nb=NUMBER  -n NUMBER    Download ecosystem by build number from NB server.\n"
    "  --install=FILE        -i FILE      Installs the ecosystem by file name on the SD card.\n"
    "  --install=NUMBER      -i NUMBER    Installs the ecosystem by build number on the SD card.\n"
    "  --list                -l           List of loaded ecosystems.\n"
    "  --list_nb             -r           List of ecosystems on the server in the NB folder.\n"
    "  --verbose             -v           Produce verbose output.\n"
    "\n";

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

bool isValidURL(const std::string& url) {
    const std::regex pattern(R"(^(https?):\/\/([^\s\/$.?#].[^\s]*\.zip)$)", std::regex_constants::icase);
    return std::regex_match(url, pattern);
}

bool isValidFileName(const std::string& url) {
    const std::regex pattern(R"(^ecosystem-\d+\.\d{2}-\d+-[a-f0-9]{9}\.zip$)", std::regex_constants::icase);
    return std::regex_match(url, pattern);
}

/** Print usage information */
auto usage(char const* progName) -> void {

    auto arr = split(std::string(progName), '/');

    std::string name = "";
    if (arr.size() > 0)
        name = arr[arr.size() - 1];

    auto n = name.c_str();

    fprintf(stderr, "%s Version: %s-%s\n", n, VERSION_STR, REVISION_STR);
    fprintf(stderr, (char*)g_format, n, n, n, n, n, n, n, n, ECOSYSTEM_DOWNLOAD_PATH);
}

auto parse(int argc, char* argv[]) -> Options {
    Options opt;
    if (argc < 2)
        return opt;
    int option_index = 0;
    int ch = -1;

    opt.error = false;

    while ((ch = getopt_long(argc, argv, (char*)optstring, (option*)long_options, &option_index)) != -1) {
        switch (ch) {

            case 'm': {
                auto files = split(optarg, ',');

                if (files.size()) {
                    opt.filesForMD5 = files;
                    opt.calcMD5 = true;
                    return opt;
                }

                fprintf(stderr, "Error key --get: %s\n", optarg);
                opt.error = true;
                return opt;
            }

            case 'd': {

                if (isValidURL(optarg)) {
                    opt.downloadURL = true;
                    opt.url = optarg;
                    break;
                }

                fprintf(stderr, "Error key --get: %s\n", optarg);
                opt.error = true;
                return opt;
            }

            case 'n': {

                if (isValidFileName(optarg)) {
                    opt.downloadNB = true;
                    opt.nbFileName = optarg;
                    break;
                } else if (std::string(optarg) != "") {
                    opt.downloadNB = true;
                    opt.nbBuildNumber = optarg;
                    break;
                }

                fprintf(stderr, "Error key --get: %s\n", optarg);
                opt.error = true;
                return opt;
            }

            case 'i': {

                if (isValidFileName(optarg)) {
                    opt.install = true;
                    opt.installFileName = optarg;
                    break;
                } else if (std::string(optarg) != "") {
                    opt.install = true;
                    opt.installNumber = optarg;
                    break;
                }

                fprintf(stderr, "Error key --get: %s\n", optarg);
                opt.error = true;
                return opt;
            }

            case 'l': {
                opt.listOflocal = true;
                return opt;
            }

            case 'r': {
                opt.listOfNB = true;
                return opt;
            }

            case 'v': {
                opt.verbose = true;
                break;
            }

            default: {
                fprintf(stderr, "[ERROR] Unknown parameter %c\n", (char)ch);
                exit(EXIT_FAILURE);
            }
        }
    }

    return opt;
}