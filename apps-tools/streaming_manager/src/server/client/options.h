#pragma once
#include <string>
#include <vector>
#include <chrono>

namespace ClientOpt {
    enum class Mode{
        ERROR_MODE,
        ERROR_PARAM,
        SEARCH,
        CONFIG,
        REMOTE,
        STREAMING,
        STREAMING_DAC,
        STREAMING_DAC_CONF
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
        START_STOP,
        START_DAC,
        STOP_DAC,
        START_STOP_DAC
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

    enum class RepeatDAC{
        NONE = -1,
        INF  = -2
    };

    struct Options {
        Mode mode;
        std::string port;
        int timeout;
        std::vector<std::string> hosts;
        ConfGet conf_get;
        ConfSet conf_set;
        std::string  conf_file;
        std::string  streaming_conf_file;
        bool verbous;
        RemoteMode remote_mode;

        // streaming
        std::string   save_dir;
        std::string   dac_file; // For DAC streaming
        int           dac_repeat; // For DAC streaming
        std::string   dac_port; // For DAC streaming
        int64_t       dac_memory; // For DAC streaming

        StreamingType streamign_type;
        SaveType      save_type;
        int           samples;
        std::string   controlPort;
        ////////////////////////

        Options(){
            mode = Mode::ERROR_MODE;
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
            dac_file = "";
            dac_repeat = (int)RepeatDAC::NONE;
            dac_port = "";
            dac_memory = 1048576;
            streaming_conf_file = "";
        };
    };

    auto usage(char const *progName) -> void;
    auto parse(int argc, char* argv[]) -> Options;
}



auto time_point_to_string(std::chrono::system_clock::time_point &tp) -> std::string;
auto getTS(std::string suffix = "") -> std::string;
