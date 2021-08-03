#pragma once

#include "options.h"
#include "ClientNetConfigManager.h"

enum class StateRunnedHosts{
    NONE,
    TCP,
    UDP,
    LOCAL
};

auto startRemote(ClientOpt::Options &option,std::map<std::string,StateRunnedHosts> *runned_hosts = nullptr) -> bool;
auto remoteSIGHandler() -> void;