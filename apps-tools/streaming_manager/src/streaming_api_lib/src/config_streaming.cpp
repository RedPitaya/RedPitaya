#include "config_streaming.h"
#include <csignal>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include "callbacks.h"
#include "common.h"
#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"

std::function<void(int)> shutdown_handler;

void signal_handler(int signal) {
    shutdown_handler(signal);
}

struct ConfigStreamClient::Impl {
    ClientNetConfigManager::Ptr m_configClient = nullptr;
    bool m_verbose = false;
    std::vector<std::shared_ptr<ConfigCallback>> m_callback;
    std::map<std::string, bool> m_terminate;
    std::mutex m_smutex;
    std::atomic<int> m_runClientCounter;
    std::vector<std::thread*> clients;
};

ConfigStreamClient::ConfigStreamClient() {
    m_pimpl = new Impl();
    m_pimpl->m_configClient = std::make_shared<ClientNetConfigManager>("", false);
    m_pimpl->m_configClient->getMemBlockSizeNofiy.connect([&](std::string host, std::string size) {
        const std::lock_guard lock(m_pimpl->m_smutex);
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s Memory block size of %s is %s\n", getTS(": ").c_str(), host.c_str(), size.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configMemoryBlockSize(this, host, atoi(size.c_str()));
    });

	m_pimpl->m_configClient->getActiveChannelsNofiy.connect([&](std::string host, adc_channels_t ac) {
		const std::lock_guard lock(m_pimpl->m_smutex);
		if (m_pimpl->m_verbose)
			aprintf(stdout, "%s Active channels %s for %s\n", getTS(": ").c_str(), ac.format().c_str(), host.c_str());
		for (auto cl : m_pimpl->m_callback) {
			std::array<bool, 4> ch;
			ch[0] = ac.isEnabled(ADCChannels::ADC_CH1);
			ch[1] = ac.isEnabled(ADCChannels::ADC_CH2);
			ch[2] = ac.isEnabled(ADCChannels::ADC_CH3);
			ch[3] = ac.isEnabled(ADCChannels::ADC_CH4);
			cl->configActiveChannels(this, host, ch);
		}
	});

	m_pimpl->m_configClient->serverStoppedNofiy.connect([&](std::string host) {
		if (m_pimpl->m_verbose)
			aprintf(stderr, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStopped(this, host);
	});

	m_pimpl->m_configClient->serverStoppedNoActiveChannelsNofiy.connect([&](std::string host) {
		if (m_pimpl->m_verbose)
            aprintf(stderr, "%s Streaming stopped: %s. No active channels [OK].\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStoppedNoActiveChannels(this, host);
	});

	m_pimpl->m_configClient->serverStoppedMemErrorNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stderr, "%s Streaming stopped: %s. Not enough DMA memory [OK].\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStoppedMemError(this, host);
    });

    m_pimpl->m_configClient->serverStoppedMemModifyNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stderr, "%s Streaming stopped: %s memory modify [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStoppedMemModify(this, host);
    });

    m_pimpl->m_configClient->serverStoppedSDFullNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stderr, "%s Streaming stopped: %s SD is full [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStoppedSDFull(this, host);
    });

    m_pimpl->m_configClient->serverStoppedSDDoneNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stderr, "%s Streaming stopped: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStoppedSDDone(this, host);
    });

    m_pimpl->m_configClient->serverDacStoppedMemErrorNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC Streaming stopped: %s memory error [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStoppedMemError(this, host);
    });

    m_pimpl->m_configClient->serverDacStoppedMemModifyNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC Streaming stopped: %s memory modify [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStoppedMemModify(this, host);
    });

    m_pimpl->m_configClient->serverDacStoppedConfigErrorNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC Streaming stopped: %s config error [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStoppedConfigError(this, host);
    });

    m_pimpl->m_configClient->serverDacStoppedSDDoneNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStoppedSDDone(this, host);
    });

    m_pimpl->m_configClient->serverDacStoppedSDEmptyNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC Streaming stopped: %s file is empty [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStoppedSDEmpty(this, host);
    });

    m_pimpl->m_configClient->serverDacStoppedSDBrokenNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC Streaming stopped: %s file is broken [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStoppedSDBroken(this, host);
    });

    m_pimpl->m_configClient->serverDacStoppedSDMissingNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC Streaming stopped: %s missing file [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStoppedSDMissing(this, host);
    });

    m_pimpl->m_configClient->configFileMissedNotify.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s Streaming started: %s config file is missed [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configErrorFileMissed(this, host);
    });

    m_pimpl->m_configClient->serverStartedTCPNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s Streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStartedTCP(this, host);
    });

    m_pimpl->m_configClient->serverStartedSDNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s Streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStartedSD(this, host);
    });

    m_pimpl->m_configClient->startADCDoneNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s ADC is run: %s\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->adcServerStartedFPGA(this, host);
    });

    m_pimpl->m_configClient->serverDacStartedNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStartedTCP(this, host);
    });

    m_pimpl->m_configClient->serverDacStartedSDNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s DAC streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->dacServerStartedSD(this, host);
    });

    m_pimpl->m_configClient->successSendConfigNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose) {
            aprintf(stdout, "%s SET: %s [OK]\n", getTS(": ").c_str(), host.c_str());
            aprintf(stdout, "%s Send configuration save command to: %s\n", getTS(": ").c_str(), host.c_str());
        }
        for (auto cl : m_pimpl->m_callback)
            cl->configSuccessSend(this, host);
    });

    m_pimpl->m_configClient->failSendConfigNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s SET: %s [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configFailSend(this, host);
    });

    m_pimpl->m_configClient->successSaveConfigNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s SAVE TO FILE: %s [OK]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configSuccessSave(this, host);
    });

    m_pimpl->m_configClient->failSaveConfigNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s SAVE TO FILE: %s [FAIL]\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configFailSave(this, host);
    });

    m_pimpl->m_configClient->getNewSettingsItemNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s Get settings from: %s\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configGetNewSettingsItem(this, host);
    });

    m_pimpl->m_configClient->getNewSettingsNofiy.connect([&](std::string host) {
        if (m_pimpl->m_verbose)
            aprintf(stdout, "%s Get settings from: %s\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configGetNewSettings(this, host);
    });

    auto sigInt = [&](int) {
        for (auto cl : m_pimpl->m_callback)
            cl->sigInt();
    };
    shutdown_handler = sigInt;
    std::signal(SIGINT, signal_handler);
}

ConfigStreamClient::~ConfigStreamClient() {
    removeCallbacks();
    delete m_pimpl;
}

auto ConfigStreamClient::connect() -> bool {
    auto host = search();
    if (host == "") {
        aprintf(stdout, "%s Host not found\n", getTS(": ").c_str());
    }
    return connect({host});
}

auto ConfigStreamClient::connect(std::vector<std::string> hosts) -> bool {
    std::atomic<int> connect_counter;
    std::atomic<uint16_t> connected;

    m_pimpl->m_configClient->serverConnectedNofiy.connect([&](std::string host) {
        const std::lock_guard lock(m_pimpl->m_smutex);
        if (m_pimpl->m_verbose)
            aprintf(stderr, "%s Connected: %s\n", getTS(": ").c_str(), host.c_str());
        for (auto cl : m_pimpl->m_callback)
            cl->configConnected(this, host);
        connect_counter--;
        connected++;
    });

    m_pimpl->m_configClient->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, error_code err) {
        const std::lock_guard lock(m_pimpl->m_smutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            for (auto cl : m_pimpl->m_callback)
                cl->configError(this, host, err.value());
            connect_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
            if (m_pimpl->m_verbose)
                aprintf(stderr, "%s Connect timeout: %s\n", getTS(": ").c_str(), host.c_str());
            for (auto cl : m_pimpl->m_callback)
                cl->configErrorTimeout(this, host);
            connect_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG) {
            if (m_pimpl->m_verbose)
                aprintf(stderr, "%s Error get settings from: %s\n", getTS(": ").c_str(), host.c_str());
            for (auto cl : m_pimpl->m_callback)
                cl->configError(this, host, err.value());
        }

        if (errors == ClientNetConfigManager::Errors::ERROR_SEND_CONFIG) {
            if (m_pimpl->m_verbose)
                aprintf(stderr, "%s Error send configuration: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            for (auto cl : m_pimpl->m_callback)
                cl->configError(this, host, err.value());
        }
    });

    connect_counter = hosts.size();
    connected = 0;
    m_pimpl->m_configClient->connectToServers(hosts, NET_CONFIG_PORT);

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto timeout = false;
    while (!timeout && connect_counter > 0) {
        sleepMs(100);
        timeout = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count() - beginTime > 5000);
    }
    return !timeout && connected == hosts.size();
}

auto ConfigStreamClient::addCallback(std::shared_ptr<ConfigCallback> callback) -> void {
    const std::lock_guard lock(m_pimpl->m_smutex);
    for (auto cl : m_pimpl->m_callback) {
        if (cl == callback)
            return;
    }
    m_pimpl->m_callback.push_back(callback);
}

auto ConfigStreamClient::removeCallbacks() -> void {
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback.clear();
}

auto ConfigStreamClient::removeCallback(std::shared_ptr<ConfigCallback> callback) -> void {
    const std::lock_guard lock(m_pimpl->m_smutex);
    m_pimpl->m_callback.erase(std::remove(m_pimpl->m_callback.begin(), m_pimpl->m_callback.end(), callback), m_pimpl->m_callback.end());
}

auto ConfigStreamClient::setVerbose(bool enable) -> void {
    m_pimpl->m_verbose = enable;
}

auto ConfigStreamClient::sendConfig(std::string key, std::string value) -> bool {
    return sendConfigCommon(this, this->m_pimpl->m_configClient, key, value, m_pimpl->m_verbose);
}

auto ConfigStreamClient::sendConfig(std::string host, std::string key, std::string value) -> bool {
    return sendConfigCommon(this, this->m_pimpl->m_configClient, host, key, value, m_pimpl->m_verbose);
}

auto ConfigStreamClient::getConfig(std::string key) -> std::string {
    return getConfigCommon(this, this->m_pimpl->m_configClient, key, m_pimpl->m_verbose);
}

auto ConfigStreamClient::getConfig(std::string host, std::string key) -> std::string {
    return getConfigCommon(this, this->m_pimpl->m_configClient, host, key, m_pimpl->m_verbose);
}

auto ConfigStreamClient::sendFileConfig(std::string config) -> bool {
    return sendFileConfigCommon(this, this->m_pimpl->m_configClient, config, m_pimpl->m_verbose);
}

auto ConfigStreamClient::sendFileConfig(std::string host, std::string config) -> bool {
    return sendFileConfigCommon(this, this->m_pimpl->m_configClient, host, config, m_pimpl->m_verbose);
}

auto ConfigStreamClient::getFileConfig() -> std::string {
    return getFileConfigCommon(this, this->m_pimpl->m_configClient, m_pimpl->m_verbose);
}

auto ConfigStreamClient::getFileConfig(std::string host) -> std::string {
    return getFileConfigCommon(this, this->m_pimpl->m_configClient, host, m_pimpl->m_verbose);
}

auto ConfigStreamClient::getHosts() -> std::list<std::string> {
    return m_pimpl->m_configClient->getHosts();
}

auto ConfigStreamClient::getModeByHost(const std::string host) -> EMode {
    return static_cast<EMode>(m_pimpl->m_configClient->getModeByHost(host));
}

auto ConfigStreamClient::requestMemoryBlockSize(const std::string host) -> bool {
    return m_pimpl->m_configClient->requestMemoryBlockSize(host);
}

auto ConfigStreamClient::requestActiveChannels(const std::string host) -> bool {
    return m_pimpl->m_configClient->requestActiveChannels(host);
}

auto ConfigStreamClient::requestADCServerStart(const std::string host) -> bool {
    return m_pimpl->m_configClient->sendADCServerStart(host);
}

auto ConfigStreamClient::requestADCServerStop(const std::string host) -> bool {
    return m_pimpl->m_configClient->sendADCServerStop(host);
}

auto ConfigStreamClient::requestADCServerFPGAStart(const std::string host) -> bool {
    return m_pimpl->m_configClient->sendADCFPGAStart(host);
}

auto ConfigStreamClient::requestDACServerStart(const std::string host, bool ch1Enable, bool ch2Enable) -> bool
{
	dac_channels_t ac;
	ac[DACChannels::DAC_CH1] = ch1Enable;
	ac[DACChannels::DAC_CH2] = ch2Enable;
	return m_pimpl->m_configClient->sendDACServerStart(host, ac);
}

auto ConfigStreamClient::requestSaveSettings(const std::string host) -> bool {
    return m_pimpl->m_configClient->sendSaveToFile(host);
}
