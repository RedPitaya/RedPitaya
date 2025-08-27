#ifndef CONFIG_H
#define CONFIG_H

#include "config_net_lib/client_net_config_manager.h"
#include "options.h"

auto sleepMs(int ms) -> void;
auto connectConfigServer(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool;
auto startConfig(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool;
auto configSIGHandler() -> void;

#endif
