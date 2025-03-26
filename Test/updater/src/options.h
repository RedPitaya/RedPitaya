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

struct Options {
    bool error = false;
    bool calcMD5 = false;
    std::vector<std::string> filesForMD5;
    bool downloadURL = false;
    std::string url = "";

    bool downloadNB = false;
    std::string nbFileName = "";
    std::string nbBuildNumber = "";

    bool install = false;
    std::string installFileName = "";
    std::string installNumber = "";

    bool listOflocal = false;
    bool listOfNB = false;

    bool verbose = false;
};

auto usage(char const* progName) -> void;
auto parse(int argc, char* argv[]) -> Options;
std::vector<std::string> split(const std::string& s, char seperator);

#endif