#pragma once

#include "options.h"
#include "config.h"

auto setOptions(ClientOpt::Options option) -> void;

auto resetStreamingCounter() -> void;
auto addStatisticSteaming(std::string &host,uint64_t bytesCount,uint64_t samp_ch1,uint64_t samp_ch2,uint64_t lost,uint64_t networkLost,uint64_t fileLost) -> void;
auto printStatisitc(bool force) -> void;

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}
