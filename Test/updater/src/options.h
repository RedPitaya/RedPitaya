#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <string>
#include <vector>

#define XSTR(s) STR(s)
#define STR(s) #s

#ifndef VERSION
#define VERSION_STR "0.00-0000"
#else
#define VERSION_STR XSTR(VERSION)
#endif

#ifndef REVISION
#define REVISION_STR "unknown"
#else
#define REVISION_STR XSTR(REVISION)
#endif

#define CRED_PATH "/opt/redpitaya/prod_cred.txt"

enum Mode { NONE = 0, MD5 = 1, DOWNLOAD = 2, DOWNLOAD_NB = 3, INSTALL = 4, LIST_LOCAL = 5, LIST_NB = 6, LIST_PROD = 7, DOWNLOAD_PROD = 8 };

struct Options {
    Mode mode = NONE;
    bool error = false;
    std::vector<std::string> filesForMD5;
    std::string url = "";

    std::string nbFileName = "";
    std::string nbBuildNumber = "";

    std::string installFileName = "";
    std::string installNumber = "";

    bool verbose = false;
    bool verbose_short = false;

    bool webcontrol = false;

    std::string user = "";
    std::string password = "";
};

auto usage(char const* progName) -> void;
auto parse(int argc, char* argv[]) -> Options;
std::vector<std::string> split(const std::string& s, char seperator);

#endif