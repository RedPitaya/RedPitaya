#include <chrono>
#include <list>
#include <map>
#include <string>

#include "file_logger.h"
#include "profiler.h"

namespace profiler {

std::map<std::string, std::chrono::system_clock::time_point> g_times;
std::map<std::string, std::list<std::string>> g_saved;
std::mutex g_mutex;

auto resetAll() -> void {
#ifdef PROFILE_ENABLED
    std::lock_guard lock(g_mutex);
    g_times.clear();
    g_saved.clear();
#endif
}

auto clearHistory(__attribute__((unused)) const std::string& name) -> void {
#ifdef PROFILE_ENABLED
    std::lock_guard lock(g_mutex);
    g_saved[name] = {};
#endif
}

auto setTimePoint(__attribute__((unused)) const std::string& name) -> void {
#ifdef PROFILE_ENABLED
    std::lock_guard lock(g_mutex);
    g_times[name] = std::chrono::system_clock::now();
#endif
}

auto saveTimePointmS(__attribute__((unused)) const std::string& name, __attribute__((unused)) const char* format, ...) -> void {
#ifdef PROFILE_ENABLED
    if (g_times.count(name)) {
        std::lock_guard lock(g_mutex);
        auto current = std::chrono::system_clock::now();
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);
        if (g_saved.count(name) == 0) {
            g_saved[name] = {};
        }
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(current - g_times[name]);
        std::string s = "[P] " + std::to_string(diff.count()) + " mS. Info: " + buffer;
        g_saved[name].push_back(s);
    } else {
        WARNING("Key not found %s", name.c_str())
    }
#endif
}

auto saveTimePointuS(__attribute__((unused)) const std::string& name, __attribute__((unused)) const char* format, ...) -> void {
#ifdef PROFILE_ENABLED
    if (g_times.count(name)) {
        std::lock_guard lock(g_mutex);
        auto current = std::chrono::system_clock::now();
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);
        if (g_saved.count(name) == 0) {
            g_saved[name] = {};
        }
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(current - g_times[name]);
        std::string s = "[P] " + std::to_string(diff.count()) + " uS. Info: " + buffer;
        g_saved[name].push_back(s);
    } else {
        WARNING("Key not found %s", name.c_str())
    }
#endif
}

auto saveTimePointnS(__attribute__((unused)) const std::string& name, __attribute__((unused)) const char* format, ...) -> void {
#ifdef PROFILE_ENABLED
    if (g_times.count(name)) {
        std::lock_guard lock(g_mutex);
        auto current = std::chrono::system_clock::now();
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);
        if (g_saved.count(name) == 0) {
            g_saved[name] = {};
        }
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(current - g_times[name]);
        std::string s = "[P] " + std::to_string(diff.count()) + " nS. Info: " + buffer;
        g_saved[name].push_back(s);
    } else {
        WARNING("Key not found %s", name.c_str())
    }
#endif
}

auto print(__attribute__((unused)) const std::string name) -> void {
#ifdef PROFILE_ENABLED
    std::lock_guard lock(g_mutex);
    if (name == "") {
        for (auto const& item : g_saved) {
            for (auto const& str : item.second) {
                printf("%s\n", str.c_str());
            }
        }
    } else {
        if (g_saved.count(name)) {
            for (auto const& str : g_saved[name]) {
                printf("%s\n", str.c_str());
            }
        }
    }
#endif
}

auto printnS(__attribute__((unused)) const std::string& name, __attribute__((unused)) const char* format, ...) -> void {
#ifdef PROFILE_ENABLED
    if (g_times.count(name)) {
        std::lock_guard lock(g_mutex);
        auto current = std::chrono::system_clock::now();
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(current - g_times[name]);
        std::string s = "[P] " + std::to_string(diff.count()) + " nS. Info: " + buffer;
        printf("%s\n", s.c_str());
    } else {
        WARNING("Key not found %s", name.c_str())
    }
#endif
}

auto printuS(__attribute__((unused)) const std::string& name, __attribute__((unused)) const char* format, ...) -> void {
#ifdef PROFILE_ENABLED
    if (g_times.count(name)) {
        std::lock_guard lock(g_mutex);
        auto current = std::chrono::system_clock::now();
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(current - g_times[name]);
        std::string s = "[P] " + std::to_string(diff.count()) + " uS. Info: " + buffer;
        printf("%s\n", s.c_str());
    } else {
        WARNING("Key not found %s", name.c_str())
    }
#endif
}

auto printmS(__attribute__((unused)) const std::string& name, __attribute__((unused)) const char* format, ...) -> void {
#ifdef PROFILE_ENABLED
    if (g_times.count(name)) {
        std::lock_guard lock(g_mutex);
        auto current = std::chrono::system_clock::now();
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(current - g_times[name]);
        std::string s = "[P] " + std::to_string(diff.count()) + " mS. Info: " + buffer;
        printf("%s\n", s.c_str());
    } else {
        WARNING("Key not found %s", name.c_str())
    }
#endif
}

}  // namespace profiler
