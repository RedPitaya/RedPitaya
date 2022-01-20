#pragma once

#include "options.h"
#include "config.h"

auto setOptions(ClientOpt::Options option) -> void;
auto getHostConfig(std::string host) -> bool;
auto helperSIGHandler() -> void;

auto resetStreamingCounter() -> void;
auto addStatisticSteaming(std::string &host,uint64_t bytesCount) -> void;
auto printStatisitc(bool force) -> void;