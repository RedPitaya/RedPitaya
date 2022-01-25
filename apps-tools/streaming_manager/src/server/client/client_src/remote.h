#pragma once

#include <map>
#include "options.h"
#include "ClientNetConfigManager.h"

enum class StateRunnedHosts{
    NONE,
    TCP,
    UDP,
    LOCAL
};

auto startRemote(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option,std::map<std::string,StateRunnedHosts> *runned_hosts = nullptr) -> bool;
auto remoteSIGHandler() -> void;
