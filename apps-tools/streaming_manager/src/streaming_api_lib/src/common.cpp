#include <chrono>
#include <memory>
#include <thread>

#include "callbacks.h"
#include "common.h"
#include "config_net_lib/client_net_config_manager.h"
#include "logger_lib/file_logger.h"

std::mutex g_rmutex;

auto getTS(std::string suffix) -> std::string {
    using namespace std;
    using namespace std::chrono;

    auto timeNow = system_clock::now();
    auto ttime_t = system_clock::to_time_t(timeNow);

    auto ms = duration_cast<milliseconds>(timeNow.time_since_epoch()) % 1000;

    std::tm* ttm = std::localtime(&ttime_t);

    char time_buffer[32];
    strftime(time_buffer, sizeof(time_buffer), "%Y.%m.%d-%H.%M.%S", ttm);

    char final_buffer[64];
    snprintf(final_buffer, sizeof(final_buffer), "%s.%03lld%s", time_buffer, (long long)ms.count(), suffix.c_str());

    return std::string(final_buffer);
}

auto sleepMs(int ms) -> void {
    std::this_thread::sleep_for(std::chrono::microseconds(1000 * ms));
}

auto search() -> std::string {
    ClientNetConfigManager client("", false);
    client.startBroadcast("127.0.0.1", NET_BROADCAST_PORT);
    int timout = 1;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
    auto value = curTime.time_since_epoch();
    auto timeBegin = value.count();
    while ((value.count() - timeBegin) < timout * 1000) {
        sleepMs(100);
        value = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch();
        auto list = client.getBroadcastClients();
        if (list.size() > 0)
            return list.front().host;
    }
    return "";
}

auto requestMemoryBlockSizeCommon(std::shared_ptr<ConfigStreamClient> cl, const std::list<std::string>& hosts, std::map<std::string, uint32_t>* sizes, bool verbose) -> bool {
    std::atomic<int> rstart_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*rstart_counter)--;
        }

        void configMemoryBlockSize(ConfigStreamClient* cl, std::string host, size_t size) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*rstart_counter)--;
            if (sizes)
                (*sizes)[host] = size;
        }

        std::atomic<int>* rstart_counter = nullptr;
        std::map<std::string, uint32_t>* sizes = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, std::map<std::string, uint32_t>* sizes_ref) : rstart_counter(counter), sizes(sizes_ref){};
    };
    auto cb = std::make_shared<LocalCb>(&rstart_counter, sizes);
    cl->addCallback(cb);

    rstart_counter = hosts.size();
    for (auto& host : hosts) {
        if (verbose)
            aprintf(stdout, "%s Request for memory block size sent : %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestMemoryBlockSize(host)) {
            rstart_counter--;
        }
    }
    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && rstart_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    cl->removeCallback(cb);
    return !timeout;
}

auto requestActiveChannelsCommon(std::shared_ptr<ConfigStreamClient> cl, const std::list<std::string>& hosts, std::map<std::string, uint32_t>* channels, bool verbose) -> bool {
    std::atomic<int> rstart_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*rstart_counter)--;
        }

        void configActiveChannels(ConfigStreamClient* cl, std::string host, size_t count) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*rstart_counter)--;
            if (channels)
                (*channels)[host] = count;
        }

        std::atomic<int>* rstart_counter = nullptr;
        std::map<std::string, uint32_t>* channels = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, std::map<std::string, uint32_t>* channels_ref) : rstart_counter(counter), channels(channels_ref){};
    };
    auto cb = std::make_shared<LocalCb>(&rstart_counter, channels);
    cl->addCallback(cb);

    rstart_counter = hosts.size();
    for (auto& host : hosts) {
        if (verbose)
            aprintf(stdout, "%s Request for active channels sent : %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestActiveChannels(host)) {
            rstart_counter--;
        }
    }
    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && rstart_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    cl->removeCallback(cb);
    return !timeout;
}

auto requestStartStreamingCommon(std::shared_ptr<ConfigStreamClient> cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts,
                                 std::map<std::string, StateRunningHosts>* runned_hosts, bool verbose) -> bool {
    std::atomic<int> rstart_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void configErrorFileMissed(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStoppedMemError(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStoppedNoActiveChannels(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStoppedMemModify(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStartedTCP(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            if (m_runned_hosts)
                (*m_runned_hosts)[host] = StateRunningHosts::TCP;
        }

        void adcServerStartedSD(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            if (m_runned_hosts)
                (*m_runned_hosts)[host] = StateRunningHosts::LOCAL;
        }

        std::atomic<int>* m_rstart_counter = nullptr;
        std::list<std::string>* m_masterHosts = nullptr;
        std::list<std::string>* m_slaveHosts = nullptr;
        std::map<std::string, StateRunningHosts>* m_runned_hosts = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, std::list<std::string>* masterHosts, std::list<std::string>* slaveHosts, std::map<std::string, StateRunningHosts>* runned_hosts)
            : m_rstart_counter(counter), m_masterHosts(masterHosts), m_slaveHosts(slaveHosts), m_runned_hosts(runned_hosts){};
    };
    auto cb = std::make_shared<LocalCb>(&rstart_counter, &masterHosts, &slaveHosts, runned_hosts);
    cl->addCallback(cb);

    rstart_counter = slaveHosts.size();
    for (auto& host : slaveHosts) {
        if (verbose)
            aprintf(stdout, "%s Send start command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestADCServerStart(host)) {
            rstart_counter--;
        }
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && rstart_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    if (timeout) {
        cl->removeCallback(cb);
        return false;
    }

    rstart_counter = masterHosts.size();
    for (auto& host : masterHosts) {
        if (verbose)
            aprintf(stdout, "%s Send start command to master board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestADCServerStart(host)) {
            rstart_counter--;
        }
    }

    beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    timeout = false;
    while (!timeout && rstart_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    cl->removeCallback(cb);
    return !timeout;
}

auto requestStopStreamingCommon(std::shared_ptr<ConfigStreamClient> cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts, bool verbose) -> bool {
    std::atomic<int> rstop_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_rmutex);
            (*m_rstop_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStopped(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstop_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStoppedSDFull(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstop_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStoppedSDDone(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstop_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        std::atomic<int>* m_rstop_counter = nullptr;
        std::list<std::string>* m_masterHosts = nullptr;
        std::list<std::string>* m_slaveHosts = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, std::list<std::string>* masterHosts, std::list<std::string>* slaveHosts)
            : m_rstop_counter(counter), m_masterHosts(masterHosts), m_slaveHosts(slaveHosts){};
    };
    auto cb = std::make_shared<LocalCb>(&rstop_counter, &masterHosts, &slaveHosts);
    cl->addCallback(cb);

    rstop_counter = masterHosts.size();
    for (auto& host : masterHosts) {
        if (verbose)
            aprintf(stdout, "%s Send stop command to master board %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestADCServerStop(host)) {
            rstop_counter--;
        }
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && rstop_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    if (timeout) {
        cl->removeCallback(cb);
        return false;
    }

    rstop_counter = slaveHosts.size();
    for (auto& host : slaveHosts) {
        if (verbose)
            aprintf(stdout, "%s Send stop command to slave board %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestADCServerStop(host)) {
            rstop_counter--;
        }
    }

    beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    timeout = false;
    while (!timeout && rstop_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    cl->removeCallback(cb);
    return !timeout;
}

auto requestStartADCCommon(std::shared_ptr<ConfigStreamClient> cl, std::list<std::string> masterHosts, std::list<std::string> slaveHosts,
                           std::map<std::string, StateRunningHosts>* runned_hosts, bool verbose) -> bool {
    std::atomic<int> rstart_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStoppedMemError(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStoppedMemModify(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            m_masterHosts->remove(host);
            m_slaveHosts->remove(host);
        }

        void adcServerStartedFPGA(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            if (m_runned_hosts)
                (*m_runned_hosts)[host] = StateRunningHosts::NONE;
        }

        std::atomic<int>* m_rstart_counter = nullptr;
        std::list<std::string>* m_masterHosts = nullptr;
        std::list<std::string>* m_slaveHosts = nullptr;
        std::map<std::string, StateRunningHosts>* m_runned_hosts = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, std::list<std::string>* masterHosts, std::list<std::string>* slaveHosts, std::map<std::string, StateRunningHosts>* runned_hosts)
            : m_rstart_counter(counter), m_masterHosts(masterHosts), m_slaveHosts(slaveHosts), m_runned_hosts(runned_hosts){};
    };
    auto cb = std::make_shared<LocalCb>(&rstart_counter, &masterHosts, &slaveHosts, runned_hosts);
    cl->addCallback(cb);

    rstart_counter = slaveHosts.size();

    for (auto& host : slaveHosts) {
        if (verbose)
            aprintf(stdout, "%s Send start ADC command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestADCServerFPGAStart(host)) {
            rstart_counter--;
        }
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && rstart_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    if (timeout) {
        cl->removeCallback(cb);
        return false;
    }

    rstart_counter = masterHosts.size();
    for (auto& host : masterHosts) {
        if (verbose)
            aprintf(stdout, "%s Send start ADC command to master board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestADCServerFPGAStart(host)) {
            rstart_counter--;
        }
    }

    beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    timeout = false;
    while (!timeout && rstart_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }
    cl->removeCallback(cb);
    return !timeout;
}

auto requestStartDACStreamingCommon(std::shared_ptr<ConfigStreamClient> cl, std::string host, uint8_t ac, StateRunningHosts* runned_host, bool verbose) -> bool {
    std::atomic<int> rstart_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_rmutex);
            (*m_rstart_counter)--;
        }

        void dacServerStoppedMemError(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
        }

        void dacServerStoppedMemModify(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
        }

        void dacServerStoppedConfigError(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
        }

        void configErrorFileMissed(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
        }

        void dacServerStartedTCP(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            if (m_runned_hosts)
                (*m_runned_hosts) = StateRunningHosts::TCP;
        }

        void dacServerStartedSD(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_rmutex);
            (*m_rstart_counter)--;
            if (m_runned_hosts)
                (*m_runned_hosts) = StateRunningHosts::LOCAL;
        }

        std::atomic<int>* m_rstart_counter = nullptr;
        StateRunningHosts* m_runned_hosts = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, StateRunningHosts* runned_hosts) : m_rstart_counter(counter), m_runned_hosts(runned_hosts){};
    };
    auto cb = std::make_shared<LocalCb>(&rstart_counter, runned_host);
    cl->addCallback(cb);

    rstart_counter = 1;
    if (verbose)
        aprintf(stdout, "%s Send start command to master board: %s\n", getTS(": ").c_str(), host.c_str());
    if (!cl->requestDACServerStart(host, ac)) {
        rstart_counter--;
    }
    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && rstart_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }

    cl->removeCallback(cb);
    return !timeout;
}
