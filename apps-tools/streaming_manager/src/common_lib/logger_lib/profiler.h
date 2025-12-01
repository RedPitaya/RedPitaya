#ifndef LOGGER_PROFILER_H
#define LOGGER_PROFILER_H

#include <string>

namespace profiler {

auto resetAll() -> void;
auto clearHistory(const std::string& name) -> void;
auto setTimePoint(const std::string& name) -> void;
auto saveTimePointnS(const std::string& name, const char* format, ...) -> void;
auto saveTimePointuS(const std::string& name, const char* format, ...) -> void;
auto saveTimePointmS(const std::string& name, const char* format, ...) -> void;
auto print(const std::string name = "") -> void;
auto printnS(const std::string& name, const char* format, ...) -> void;
auto printuS(const std::string& name, const char* format, ...) -> void;
auto printmS(const std::string& name, const char* format, ...) -> void;
}  // namespace profiler

#endif
