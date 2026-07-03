#ifndef CONFIG_H
#define CONFIG_H

#include <memory>
#include <string>
#include "config_net_lib/client_net_config_manager.h"

class ConfigStreamClient;

auto sendConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string key, std::string value, bool verbose) -> bool;
auto sendConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, std::string key, std::string value, bool verbose) -> bool;
auto getConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string key, bool verbose) -> std::string;
auto getConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, std::string key, bool verbose) -> std::string;

auto sendFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string config, bool verbose) -> bool;
auto sendFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, std::string config, bool verbose) -> bool;
auto getFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, bool verbose) -> std::string;
auto getFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, bool verbose) -> std::string;

#endif
