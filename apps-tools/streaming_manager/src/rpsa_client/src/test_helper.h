#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "options.h"
#include "config.h"

auto setOptions(ClientOpt::Options option) -> void;

auto resetStreamingCounter() -> void;
auto addStatisticSteaming(std::string &host,uint64_t bytesCount,uint64_t samp_ch1,uint64_t samp_ch2,uint64_t samp_ch3,uint64_t samp_ch4,uint64_t lost,uint64_t networkLost,uint64_t fileLost,int64_t brokenBuff) -> void;
auto printStatisitc(bool force) -> void;
auto printFinalStatisitc() -> void;
auto testBuffer(uint8_t *buff_c1,uint8_t *buff_c2,uint8_t *buff_c3,uint8_t *buff_c4,size_t size_ch1,size_t size_ch2,size_t size_ch3,size_t size_ch4) -> bool;

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

#endif
