#ifndef CONFIG_STREAMING_H
#define CONFIG_STREAMING_H

#include <list>
#include <memory>
#include <string>
#include <vector>

class ConfigCallback;

class ConfigStreamClient {
   public:
    enum EMode { AB_SERVER_MASTER = 0, AB_SERVER_SLAVE = 1, AB_CLIENT = 2, AB_NONE = 3 };

    ConfigStreamClient();
    ~ConfigStreamClient();

    auto connect() -> bool;
    auto connect(std::vector<std::string> hosts) -> bool;

    auto getHosts() -> std::list<std::string>;
    auto getModeByHost(const std::string host) -> EMode;

    auto requestSaveSettings(const std::string host) -> bool;
    auto requestMemoryBlockSize(const std::string host) -> bool;
    auto requestActiveChannels(const std::string host) -> bool;
    auto requestADCServerStart(const std::string host) -> bool;
    auto requestADCServerStop(const std::string host) -> bool;
    auto requestADCServerFPGAStart(const std::string host) -> bool;
    auto requestDACServerStart(const std::string host, uint8_t channels) -> bool;

    auto sendConfig(std::string key, std::string value) -> bool;
    auto sendConfig(std::string host, std::string key, std::string value) -> bool;
    auto getConfig(std::string key) -> std::string;
    auto getConfig(std::string host, std::string key) -> std::string;

    auto sendFileConfig(std::string config) -> bool;
    auto sendFileConfig(std::string host, std::string config) -> bool;
    auto getFileConfig() -> std::string;
    auto getFileConfig(std::string host) -> std::string;

    auto setVerbose(bool enable) -> void;

    auto addCallback(std::shared_ptr<ConfigCallback> callback) -> void;
    auto removeCallbacks() -> void;
    auto removeCallback(std::shared_ptr<ConfigCallback> callback) -> void;

   private:
    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

#endif
