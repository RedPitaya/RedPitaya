#pragma once
#include <string>

namespace ClientOpt {
    enum class Mode{
        ERROR,
        ERROR_PARAM,
        SEARCH,
        CONFIG,
        STREAMING
    };

    struct Optons {
        Mode mode;
        std::string port;
        int timeout;
        Optons(){
            mode = Mode::ERROR;
            port = "";
            timeout = 5;
        };
    };

    auto usage(char const *progName) -> void;
    auto parse(int argc, char* argv[]) -> Optons;
}