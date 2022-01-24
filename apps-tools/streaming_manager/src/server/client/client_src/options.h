#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <unistd.h>

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

    enum class TestMode{
        NONE   = 0,
        ENABLE = 1        
    };

    enum class TestSteamingMode{
        NONE           = 0,
        WITH_TEST_DATA = 1,
        WITH_SAVE_FILE = 2
    };


    struct Ports{
        std::string streaming_port     = "8900";
        std::string config_port        = "8901";
        std::string broadcast_port     = "8902";
        std::string dac_streaming_port = "8903";
    };

    struct Options {
        Mode mode;
        TestMode testmode;
        TestSteamingMode testStreamingMode;

        int timeout;
        std::vector<std::string> hosts;
        Ports ports;

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
        int64_t       dac_memory; // For DAC streaming

        StreamingType streamign_type;
        SaveType      save_type;
        int           samples;
        ////////////////////////

        Options(){
            mode = Mode::ERROR_MODE;
            testmode = TestMode::NONE;
            testStreamingMode = TestSteamingMode::NONE;
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
            dac_file = "";
            dac_repeat = (int)RepeatDAC::NONE;
            dac_memory = 1048576;
            streaming_conf_file = "";
        };
    };

    auto usage(char const *progName) -> void;
    auto parse(int argc, char* argv[]) -> Options;
}



auto time_point_to_string(std::chrono::system_clock::time_point &tp) -> std::string;
auto getTS(std::string suffix = "") -> std::string;
