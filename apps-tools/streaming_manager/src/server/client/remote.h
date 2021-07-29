#pragma once

#include "options.h"
#include "ClientNetConfigManager.h"

auto startRemote(ClientOpt::Options &option) -> void;
auto remoteSIGHandler() -> void;