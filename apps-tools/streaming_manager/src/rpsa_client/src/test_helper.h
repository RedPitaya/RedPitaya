#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <sstream>
#include "options.h"

auto setOptions(ClientOpt::Options option) -> void;

auto resetStreamingCounter() -> void;
auto addStatisticStreaming(std::string& host, uint64_t bytesCount, uint64_t samp_ch1, uint64_t samp_ch2, uint64_t samp_ch3, uint64_t samp_ch4, uint64_t lost, uint64_t fileLost,
                           int64_t brokenBuff) -> void;
auto printStatisitc(bool force) -> void;
auto printFinalStatisitc() -> void;
// One line of all the boards: samples per channel, data, rate, losses (the totals so far)
auto progressLine() -> std::string;
// The result of a board, one line starting with "Done:" ("Stopped:" when it ended too early)
auto resultLine(const std::string& host, const std::string& file, bool ok = true) -> std::string;
// Why a board ended a stream this client did not stop: taken over by another client or stopped
auto endedText(bool configConnected) -> const char*;
// "12.3 MB", "512 kB"
auto sizeText(uint64_t bytes) -> std::string;
// Writes a progress line over the last one when stdout is a terminal (and not --quiet or -v);
// done: clears it
auto showProgress(const std::string& line, bool done = false) -> void;
auto progressWanted(const ClientOpt::Options& option) -> bool;
auto testBuffer(uint8_t* buff_c1, uint8_t* buff_c2, uint8_t* buff_c3, uint8_t* buff_c4, size_t size_ch1, size_t size_ch2, size_t size_ch3, size_t size_ch4) -> bool;

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

#endif
