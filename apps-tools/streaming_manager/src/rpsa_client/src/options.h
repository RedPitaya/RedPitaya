#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>
#include <chrono>
#include <unistd.h>

enum class StateRunnedHosts {
	NONE,
	TCP,
	LOCAL
};

namespace ClientOpt {
enum class Mode {
	ERROR_MODE,
	ERROR_PARAM,
	SEARCH,
	CONFIG,
	REMOTE,
	STREAMING,
	STREAMING_DAC,
	CONFIG_ITEM
};

enum class ConfGet {
	NONE,
	VERBOUS_JSON,
	VERBOUS_JSON_DATA,
	VERBOUS,
	FILE
};

enum class ConfSet {
	NONE,
	MEMORY,
	FILE
};

enum class RemoteMode {
	NONE,
	START,
	START_WITH_ADC,
	STOP,
	START_STOP,
	START_DAC,
	STOP_DAC,
	START_STOP_DAC,
	START_FPGA_ADC,
	START_FPGA_DAC
};

enum class StreamingType {
	NONE,
	TDMS,
	WAV,
	CSV,
	BIN
};

enum class SaveType {
	NONE,
	RAW,
	VOL
};

enum class RepeatDAC {
	NONE = -1,
	INF = -2
};

// enum class TestMode{
//     NONE   = 0,
//     ENABLE = 1
// };

struct Options
{
	Mode mode{Mode::ERROR_MODE};

	int timeout{5};
	std::vector<std::string> hosts{};

	ConfGet conf_get{ConfGet::NONE};
	ConfSet conf_set{ConfSet::NONE};
	std::string conf_file{""};
	std::string streaming_conf_file{""};
	bool verbous{false};
	RemoteMode remote_mode{RemoteMode::NONE};

	// streaming
	std::string save_dir{""};
	std::string dac_file{""};			   // For DAC streaming
	int dac_repeat{(int) RepeatDAC::NONE}; // For DAC streaming
	int64_t dac_memory{1048576};		   // For DAC streaming

	StreamingType streamign_type;
	SaveType save_type{SaveType::NONE};
	int samples{0};
	std::string config_item_name{""};
	std::string config_item_value{""};
	bool config_item_write{false};
};

auto usage(char const *progName) -> void;
auto parse(int argc, char *argv[]) -> Options;
} // namespace ClientOpt

auto time_point_to_string(std::chrono::system_clock::time_point &tp) -> std::string;
auto getTS(std::string suffix = "") -> std::string;

#endif
