#include <chrono>

#include "callbacks.h"
#include "common.h"
#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include "logger_lib/file_logger.h"

std::mutex g_smutex;

auto sendConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string key, std::string value, bool verbose) -> bool {
    auto connected_hosts = cl->getHosts();
    if (connected_hosts.size() != 1) {
        const char* msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
        aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
        return false;
    } else {
        return sendConfigCommon(cl, cl2, connected_hosts.front(), key, value, verbose);
    };
}

auto sendConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, std::string key, std::string value, bool verbose) -> bool {
    std::atomic<int> set_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        void configSuccessSend(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            cl->requestSaveSettings(host);
        }

        void configFailSend(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        void configSuccessSave(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        void configFailSave(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        std::atomic<int>* m_set_counter = nullptr;
        bool m_verbose = false;

       public:
        explicit LocalCb(std::atomic<int>* counter, bool verbose) : m_set_counter(counter), m_verbose(verbose){};
    };
    auto cb = std::make_shared<LocalCb>(&set_counter, verbose);
    cl->addCallback(cb);

    set_counter = 1;
    if (verbose)
        aprintf(stdout, "%s Send configuration to: %s\n", getTS(": ").c_str(), host.c_str());
    if (!cl2->sendConfigVariable(host, key, value)) {
        set_counter--;
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && set_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }
    cl->removeCallback(cb);
    return !timeout;
}

auto getConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string key, bool verbose) -> std::string {
    auto connected_hosts = cl->getHosts();
    if (connected_hosts.size() != 1) {
        const char* msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
        aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
        return "";
    } else {
        return getConfigCommon(cl, cl2, connected_hosts.front(), key, verbose);
    };
}

auto getConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, std::string key, bool verbose) -> std::string {
    std::string config;
    std::atomic<int> get_counter;
    std::atomic_bool noError;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_smutex);
            *m_noError = false;
            (*m_get_counter)--;
        }

        void configGetNewSettingsItem(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            *m_noError = true;
            (*m_get_counter)--;
        }

        std::atomic<int>* m_get_counter = nullptr;
        std::atomic_bool* m_noError = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, std::atomic_bool* noError) : m_get_counter(counter), m_noError(noError){};
    };
    auto cb = std::make_shared<LocalCb>(&get_counter, &noError);
    cl->addCallback(cb);

    get_counter = 1;
    if (verbose)
        aprintf(stdout, "%s Send configuration request: %s\n", getTS(": ").c_str(), host.c_str());
    if (!cl2->requestConfigVariable(host, key)) {
        get_counter--;
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && get_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }
    cl->removeCallback(cb);
    CStreamSettings* s = cl2->getLocalSettingsOfHost(host);
    config = noError ? s->getValue(key) : "";
    return config;
}

auto sendFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string config, bool verbose) -> bool {
    auto connected_hosts = cl->getHosts();
    if (connected_hosts.size() != 1) {
        const char* msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
        aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
        return false;
    } else {
        return sendFileConfigCommon(cl, cl2, connected_hosts.front(), config, verbose);
    }
}

auto sendFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, std::string config, bool verbose) -> bool {
    if (!cl2->parseJson(config)) {
        aprintf(stdout, "%s Error applying settings for host: %s\n", getTS(": ").c_str(), host.c_str());
        return false;
    }

    std::atomic<int> set_counter;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        void configSuccessSend(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            cl->requestSaveSettings(host);
        }

        void configFailSend(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        void configSuccessSave(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        void configFailSave(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            if (m_set_counter)
                (*m_set_counter)--;
        }

        std::atomic<int>* m_set_counter = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter) : m_set_counter(counter){};
    };
    auto cb = std::make_shared<LocalCb>(&set_counter);
    cl->addCallback(cb);

    set_counter = 1;
    if (verbose)
        aprintf(stdout, "%s Send configuration to: %s\n", getTS(": ").c_str(), host.c_str());
    if (!cl2->sendConfig(host)) {
        set_counter--;
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && set_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }
    cl->removeCallback(cb);
    return !timeout;
}

auto getFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, bool verbose) -> std::string {
    auto connected_hosts = cl->getHosts();
    if (connected_hosts.size() != 1) {
        const char* msg = connected_hosts.size() == 0 ? "The server is not connected" : "More than 1 server connected";
        aprintf(stderr, "%s %s\n", getTS(": ").c_str(), msg);
        return "";
    } else {
        return getFileConfigCommon(cl, cl2, connected_hosts.front(), verbose);
    }
}

auto getFileConfigCommon(ConfigStreamClient* cl, ClientNetConfigManager::Ptr cl2, std::string host, bool verbose) -> std::string {
    std::atomic<int> get_counter;
    std::string config = "";
    std::atomic_bool noError = false;

    class LocalCb : public ConfigCallback {

        void configError(ConfigStreamClient* cl, std::string host, int error) override {
            const std::lock_guard lock(g_smutex);
            *m_noError = false;
            (*m_get_counter)--;
        }

        void configGetNewSettings(ConfigStreamClient* cl, std::string host) override {
            const std::lock_guard<std::mutex> lock(g_smutex);
            *m_noError = true;
            (*m_get_counter)--;
        }

        std::atomic<int>* m_get_counter = nullptr;
        std::atomic_bool* m_noError = nullptr;

       public:
        explicit LocalCb(std::atomic<int>* counter, std::atomic_bool* noError) : m_get_counter(counter), m_noError(noError){};
    };
    auto cb = std::make_shared<LocalCb>(&get_counter, &noError);
    cl->addCallback(cb);

    get_counter = 1;
    if (verbose)
        aprintf(stdout, "%s Send configuration request: %s\n", getTS(": ").c_str(), host.c_str());
    if (!cl2->requestConfig(host)) {
        get_counter--;
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && get_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }
    cl->removeCallback(cb);
    CStreamSettings* s = cl2->getLocalSettingsOfHost(host);
    config = noError ? s->toJson() : "";
    return config;
}
