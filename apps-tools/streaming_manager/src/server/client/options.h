#pragma once
#include <string>
#include <vector>

namespace ClientOpt {
    enum class Mode{
        ERROR,
        ERROR_PARAM,
        SEARCH,
        CONFIG,
        REMOTE,
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

    enum class RemoteMode{
        NONE,
        START,
        STOP,
        START_STOP
    };

    enum class StreamingType{
        NONE,
        TDMS,
        WAV,
        CSV
    };

    enum class SaveType{
        NONE,
        RAW,
        VOL
    };

    struct Options {
        Mode mode;
        std::string port;
        int timeout;
        std::vector<std::string> hosts;
        ConfGet conf_get;
        ConfSet conf_set;
        std::string  conf_file;
        bool verbous;
        RemoteMode remote_mode;

        // streaming
        std::string  save_dir;
        StreamingType streamign_type;
        SaveType      save_type;
        int           samples;
        std::string   controlPort;
        ////////////////////////

        Options(){
            mode = Mode::ERROR;
            port = "";
            timeout = 5;
            hosts.clear();
            conf_get = ConfGet::NONE;
            conf_set = ConfSet::NONE;
            conf_file = "";
            verbous = false;
            remote_mode = RemoteMode::NONE;
            save_dir = "";
            streamign_type = StreamingType::NONE;
            save_type = SaveType::NONE;
            samples = -1;
            controlPort = "";
        };
    };

    auto usage(char const *progName) -> void;
    auto parse(int argc, char* argv[]) -> Options;
}