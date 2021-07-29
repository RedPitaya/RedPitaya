#pragma once
#include <string>
#include <vector>

namespace ClientOpt {
    enum class Mode{
        ERROR,
        ERROR_PARAM,
        SEARCH,
        CONFIG,
        STREAMING
    };

    enum class ConfGet{
        NONE,
        VERBOUS_JSON,
        VERBOUS_JSON_DATA,
        VERBOUS,
        FILE
    };

    enum class ConfSet{
        NONE,
        MEMORY,
        FILE
    };

    struct Options {
        Mode mode;
        std::string port;
        int timeout;
        std::vector<std::string> hosts;
        ConfGet conf_get;
        ConfSet conf_set;
        std::string  conf_file;
        bool conf_verbous;
        Options(){
            mode = Mode::ERROR;
            port = "";
            timeout = 5;
            hosts.clear();
            conf_get = ConfGet::NONE;
            conf_set = ConfSet::NONE;
            conf_file = "";
            conf_verbous = false;
        };
    };

    auto usage(char const *progName) -> void;
    auto parse(int argc, char* argv[]) -> Options;
}