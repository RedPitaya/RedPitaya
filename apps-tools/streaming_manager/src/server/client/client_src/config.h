#pragma once

#include "options.h"
#include "ClientNetConfigManager.h"

auto sleepMs(int ms) -> void;
auto connectConfigServer(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) -> bool;
auto startConfig(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) -> bool;
auto configSIGHandler() -> void;
