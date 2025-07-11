#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "config_net_lib/client_net_config_manager.h"

auto sendConfigCommon(ClientNetConfigManager::Ptr cl, std::string key, std::string value, bool verb) -> bool;
auto sendConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string key, std::string value, bool verb) -> bool;
auto getConfigCommon(ClientNetConfigManager::Ptr cl, std::string key, bool verb) -> std::string;
auto getConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string key, bool verb) -> std::string;

auto sendFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string config, bool verb) -> bool;
auto sendFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string config, bool verb) -> bool;
auto getFileConfigCommon(ClientNetConfigManager::Ptr cl, bool verb) -> std::string;
auto getFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, bool verb) -> std::string;

#endif
