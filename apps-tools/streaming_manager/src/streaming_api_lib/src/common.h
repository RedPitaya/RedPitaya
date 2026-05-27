#ifndef COMMON_H
#define COMMON_H

#include "config_streaming.h"
#include "reader_controller.h"
#include <map>
#include <string>

enum class StateRunningHosts { NONE, TCP, LOCAL };

auto getTS(std::string suffix) -> std::string;
auto sleepMs(int ms) -> void;
auto search() -> std::string;
auto requestMemoryBlockSizeCommon(std::shared_ptr<ConfigStreamClient> cl, const std::list<std::string>& hosts, std::map<std::string, uint32_t>* sizes, bool verbose) -> bool;
auto requestActiveChannelsCommon(std::shared_ptr<ConfigStreamClient> cl,
								 const std::list<std::string> &hosts,
								 std::map<std::string, adc_channels_t> *channels,
								 bool verbose) -> bool;
auto requestStartStreamingCommon(std::shared_ptr<ConfigStreamClient> cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts,
                                 std::map<std::string, StateRunningHosts>* runned_hosts, bool verbose) -> bool;
auto requestStartADCCommon(std::shared_ptr<ConfigStreamClient> cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts,
                           std::map<std::string, StateRunningHosts>* runned_hosts, bool verbose) -> bool;

auto requestStopStreamingCommon(std::shared_ptr<ConfigStreamClient> cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts, bool verbose) -> bool;

auto requestStartDACStreamingCommon(
	std::shared_ptr<ConfigStreamClient> cl, std::string host, dac_channels_t ac, StateRunningHosts *runned_host, bool verbose) -> bool;
#endif
