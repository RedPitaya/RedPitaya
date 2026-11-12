#include "asio_common.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef __linux__
#include <dirent.h>
#include <pthread.h>
#include <sched.h>
#endif
#include "data_lib/neon_asm.h"

using namespace net_lib;

auto net_lib::createBuffer(const char* buffer, size_t size) -> net_buffer {
    try {
        if ((uint64_t)std::numeric_limits<size_t>::max() <= size)
            return net_buffer(nullptr);
        ;
        auto p = std::shared_ptr<uint8_t[]>(new uint8_t[static_cast<size_t>(size)]);
        memcpy_neon(p.get(), buffer, size);
        return p;
    } catch (...) {
        return net_buffer(nullptr);
    }
}

auto net_lib::createBuffer(uint64_t size) -> net_buffer {
    try {
        if ((uint64_t)std::numeric_limits<size_t>::max() <= size)
            return net_buffer(nullptr);
        ;
        auto p = std::shared_ptr<uint8_t[]>(new uint8_t[static_cast<size_t>(size)]);
        return p;
    } catch (...) {
        return net_buffer(nullptr);
        ;
    }
}

auto net_lib::senderCpu() -> int {
#ifdef __linux__
    if (const char* env = std::getenv("RP_STREAMING_SENDER_CPU"))
        return std::atoi(env) < 0 ? -1 : std::atoi(env);
    std::set<std::string> ifaces;
    if (DIR* d = opendir("/sys/class/net")) {
        while (dirent* e = readdir(d))
            if (e->d_name[0] != '.' && std::strcmp(e->d_name, "lo") != 0)
                ifaces.insert(e->d_name);
        closedir(d);
    }
    std::ifstream f("/proc/interrupts");
    std::string line;
    if (ifaces.empty() || !std::getline(f, line))
        return -1;
    size_t cpus = 0;
    {
        std::istringstream head(line);
        std::string w;
        while (head >> w)
            cpus++;
    }
    if (cpus < 2)
        return -1;
    std::vector<unsigned long long> counts(cpus, 0);
    bool found = false;
    while (std::getline(f, line)) {
        std::istringstream in(line);
        std::string w;
        std::vector<unsigned long long> row;
        in >> w;  // "41:"
        for (size_t i = 0; i < cpus && in >> w; i++)
            row.push_back(std::strtoull(w.c_str(), nullptr, 10));
        bool net = false;
        while (in >> w)
            net = net || ifaces.count(w);
        if (!net || row.size() != cpus)
            continue;
        found = true;
        for (size_t i = 0; i < cpus; i++)
            counts[i] += row[i];
    }
    if (!found)
        return -1;
    size_t best = 0;
    for (size_t i = 1; i < cpus; i++)
        if (counts[i] < counts[best])
            best = i;
    return static_cast<int>(best);
#else
    return -1;
#endif
}

auto net_lib::pinSenderThread() -> int {
#ifdef __linux__
    const int cpu = senderCpu();
    if (cpu < 0 || cpu >= CPU_SETSIZE)
        return -1;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    return pthread_setaffinity_np(pthread_self(), sizeof(set), &set) == 0 ? cpu : -1;
#else
    return -1;
#endif
}
