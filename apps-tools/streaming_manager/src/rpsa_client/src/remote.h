#ifndef REMOTE_H
#define REMOTE_H

#include <map>
#include "options.h"
#include "config_net_lib/client_net_config_manager.h"

auto startRemote(std::shared_ptr<ClientNetConfigManager> cl,
				 ClientOpt::Options &option,
				 std::map<std::string, uint8_t> *activeChannels = nullptr,
				 std::map<std::string, StateRunnedHosts> *runned_hosts = nullptr) -> bool;
auto remoteSIGHandler() -> void;
auto requestMemoryBlockSize(std::shared_ptr<ClientNetConfigManager> cl,
							std::list<std::string> &hosts,
							std::map<std::string, uint32_t> *sizes,
							bool verbose) -> bool;
auto requestActiveChannels(std::shared_ptr<ClientNetConfigManager> cl,
						   std::list<std::string> &hosts,
						   std::map<std::string, uint32_t> *sizes,
						   bool verbose) -> bool;
#endif
