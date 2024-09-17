#ifndef OPTIONS_H
#define OPTIONS_H

#include "broadcast_lib/asio_broadcast_socket.h"
#include "logger_lib/file_logger.h"
#include <string>
#include <vector>

#ifndef _WIN32
#include <syslog.h>
#define SYS(X, ...) syslog(X, __VA_ARGS__);
#else
#define SYS(X, ...)
#endif

#define MAX_ADC_CHANNELS 4
#define MAX_DAC_CHANNELS 2

#define printWithLog(X, Y, ...) \
	aprintf(Y, __VA_ARGS__); \
	SYS(X, __VA_ARGS__);

namespace ClientOpt {

typedef enum {
	RP_125_14,
	RP_250_12,
	RP_125_14_4CH
} models_t;

struct Options
{
	bool background;
	std::string conf_file;
	bool verbose;

	Options()
	{
		verbose = false;
		background = false;
#ifdef RP_PLATFORM
		conf_file = std::string("/root/.config/redpitaya/apps/streaming/streaming_config.json");
#else
		conf_file = std::string("./server.conf");
#endif
	};
};
auto split(const std::string &s, char seperator) -> std::vector<std::string>;
auto usage(char const *progName) -> void;
auto parse(int argc, char *argv[]) -> Options;

auto getADCChannels() -> uint8_t;
auto getADCBits() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getModel() -> models_t;
auto getBroadcastModel() -> broadcast_lib::EModel;
} // namespace ClientOpt

#endif
