#ifndef OPTIONS_H
#define OPTIONS_H

#include <unistd.h>
#include <chrono>
#include <string>
#include <vector>

enum class StateRunnedHosts { NONE, TCP, LOCAL };

namespace ClientOpt {
enum class Mode { ERROR_MODE, ERROR_PARAM, SEARCH, CONFIG, REMOTE, STREAMING, STREAMING_DAC, CONFIG_ITEM, SHOW_HELP, SHOW_VERSION, GPIO };

enum class ConfGet { NONE, VERBOUS_JSON, VERBOUS_JSON_DATA, VERBOUS, FILE };

enum class ConfSet { NONE, MEMORY, FILE };

enum class RemoteMode { NONE, START, START_WITH_ADC, STOP, START_STOP, START_DAC, STOP_DAC, START_STOP_DAC, START_FPGA_ADC, START_FPGA_DAC,
                        START_GPIO_IN, STOP_GPIO_IN, START_GPIO_OUT, STOP_GPIO_OUT, WATCH, STATUS };

enum class StreamingType { NONE, TDMS, WAV, CSV, BIN };

enum class SaveType { NONE, RAW, VOL };

enum class RepeatDAC { INF = -1 };

// enum class TestMode{
//     NONE   = 0,
//     ENABLE = 1
// };

struct Options {
    Mode mode{Mode::ERROR_MODE};

    int timeout{5};             // detect, remote, streaming: milliseconds
    int reply_timeout_ms{10000};  // how long an answer of a board is waited for
    bool quiet{false};            // only the errors and the result
    bool json{false};             // --json: the events as JSON Lines on stdout (streaming_improvements 4.13)
    std::vector<std::string> hosts{};

    ConfGet conf_get{ConfGet::NONE};
    ConfSet conf_set{ConfSet::NONE};
    std::string conf_file{""};
    std::string streaming_conf_file{""};
    bool verbous{false};
    RemoteMode remote_mode{RemoteMode::NONE};

    // streaming
    std::string save_dir{""};
    std::string dac_file{""};  // For DAC streaming
    int64_t dac_repeat{1};     // For DAC streaming

    StreamingType streamign_type{StreamingType::NONE};
    SaveType save_type{SaveType::NONE};
    uint64_t samples{0};  // per channel; 0 - no limit
    // streaming_improvements 4.11: the files here; 0 - none
    double limit_time{0};     // seconds of the time line of the stream
    uint64_t limit_size{0};   // bytes of data
    uint64_t split_size{0};   // parts of bytes
    double split_time{0};     // parts of seconds
    bool tdms_time{false};  // a TDMS column with the time of every sample
    // -i, as many as given, in their order: a read (NAME) or a write (NAME=VALUE, cut at the
    // first "=", the value may hold more of them)
    struct ConfigItem {
        std::string name;
        std::string value;
        bool write{false};
    };
    std::vector<ConfigItem> config_items;
    // the -i being done now
    std::string config_item_name{""};
    std::string config_item_value{""};
    bool config_item_write{false};

    // GPIO: the input into this file, this file on the output
    std::string gpio_in_file{""};
    std::string gpio_out_file{""};
    std::string gpio_format{""};  // of the input file: bin, csv, vcd; "" - its extension
    bool gpio_gaps{false};         // the output plays the gaps of a recording

    // SHOW_HELP: the section and the filter of the keys; ERROR_PARAM: the section of the mode
    std::string help_topic{""};
    std::string help_filter{""};
};

auto parse(int argc, char* argv[]) -> Options;
}  // namespace ClientOpt

auto time_point_to_string(std::chrono::system_clock::time_point& tp) -> std::string;
auto getTS(std::string suffix = "") -> std::string;

#endif
