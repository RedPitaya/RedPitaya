#ifndef COMMON_H
#define COMMON_H

#include <string>
#include "config_net_lib/client_net_config_manager.h"

enum class StateRunningHosts { NONE, TCP, LOCAL };

auto getTS(std::string suffix) -> std::string;
auto sleepMs(int ms) -> void;
auto search() -> std::string;
auto requestMemoryBlockSize(ClientNetConfigManager::Ptr cl, const std::list<std::string>& hosts, std::map<std::string, uint32_t>* sizes, bool verbose) -> bool;
auto requestActiveChannels(ClientNetConfigManager::Ptr cl, const std::list<std::string>& hosts, std::map<std::string, uint32_t>* channels, bool verbose) -> bool;
auto requestStartStreaming(ClientNetConfigManager::Ptr cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts,
                           std::map<std::string, StateRunningHosts>* runned_hosts, bool verbose) -> bool;
auto requestStartADC(ClientNetConfigManager::Ptr cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts, std::map<std::string, StateRunningHosts>* runned_hosts,
                     bool verbose) -> bool;

auto requestStopStreaming(ClientNetConfigManager::Ptr cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts, bool verbose) -> bool;

auto requestStartDACStreaming(ClientNetConfigManager::Ptr cl, std::string host, uint8_t ac, StateRunningHosts* runned_host, bool verbose) -> bool;
#endif
