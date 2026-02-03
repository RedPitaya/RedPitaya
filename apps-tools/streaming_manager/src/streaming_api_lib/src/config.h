#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "config_net_lib/client_net_config_manager.h"

auto sendConfigCommon(ClientNetConfigManager::Ptr cl, std::string key, std::string value, bool verbose) -> bool;
auto sendConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string key, std::string value, bool verbose) -> bool;
auto getConfigCommon(ClientNetConfigManager::Ptr cl, std::string key, bool verbose) -> std::string;
auto getConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string key, bool verbose) -> std::string;

auto sendFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string config, bool verbose) -> bool;
auto sendFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, std::string config, bool verbose) -> bool;
auto getFileConfigCommon(ClientNetConfigManager::Ptr cl, bool verbose) -> std::string;
auto getFileConfigCommon(ClientNetConfigManager::Ptr cl, std::string host, bool verbose) -> std::string;

#endif
