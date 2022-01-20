#pragma once
#include <string>
#include <vector>
#include <chrono>

namespace ClientOpt {
    enum class State{
        ERROR_PARAM,
        TEST
    };

    enum class ConfGet{
        NONE,
        VERBOUS_JSON,
        VERBOUS_JSON_DATA,
        VERBOUS,
        FILE
    };

    enum class Mode{
        DD
    };

    enum class Channels{
        ONE,
        TWO
    };

    struct Ports{
        std::string streaming_port     = "8900";
        std::string config_port        = "8901";
        std::string dac_streaming_port = "8903";
    };

    struct Options {
        State state;
        int timeout;
        std::string host;
        Ports ports;
        bool verbous;

        Mode mode;
        int  speed;
        Channels chs;

        Options(){
            state = State::ERROR_PARAM;
            timeout = 10;
            host  = "";
            verbous = false;
            mode = Mode::DD;
            chs = Channels::TWO;
            speed = -1;
        };
    };

    auto usage(char const *progName) -> void;
    auto parse(int argc, char* argv[]) -> Options;
}



auto time_point_to_string(std::chrono::system_clock::time_point &tp) -> std::string;
auto getTS(std::string suffix = "") -> std::string;
