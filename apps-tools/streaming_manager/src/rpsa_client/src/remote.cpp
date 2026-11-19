#include "remote.h"
#include <mutex>
#include "config.h"
#include "records.h"
#include "logger_lib/file_logger.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

ClientOpt::Options g_roption;
//std::atomic<int>   g_rstart_counter;
//std::atomic<int>   g_rstop_counter;
std::mutex g_rmutex;
std::atomic<bool> g_rexit_flag;

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                    std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool;
auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                       std::map<std::string, dac_channels_t>* activeChannels, std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool;
auto stopStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                   std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool;
auto stopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                      std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool;
auto startStopStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                        std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool;
auto startStopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                           std::map<std::string, dac_channels_t>* activeChannels, std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool;
auto startADC(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
              std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool;

// The GPIO input or output of the boards started or stopped (for the file mode of the board or
// another client); every board's answer is told
static auto remoteGpio(std::shared_ptr<ClientNetConfigManager> cl, const std::list<std::string>& hosts, bool start, bool output) -> bool {
    std::atomic<int> count{0};
    std::atomic<bool> failed{false};
    std::mutex m;
    const char* dir = output ? "output" : "input";
    auto answer = [&](const std::string& host, const std::string& text, bool ok) {
        const std::lock_guard lock(m);
        aprintf(ok ? stdout : stderr, "%s %s%s: GPIO %s %s\n", getTS(": ").c_str(), ok ? "" : "[ERROR] ", host.c_str(), dir, text.c_str());
        if (!ok)
            failed = true;
        count--;
    };
    if (output) {
        cl->serverGpioOutStartedNofiy.connect([&](std::string h) { answer(h, "started, it waits for a client's data", start); });
        cl->serverGpioOutStartedSDNofiy.connect([&](std::string h) { answer(h, "started, it plays its file (gpio_out_file_sd)", start); });
        cl->serverGpioOutStoppedNofiy.connect([&](std::string h, uint64_t w) { answer(h, "stopped, it waited " + std::to_string(w) + " clocks for data", !start); });
        cl->serverGpioOutStoppedSDDoneNofiy.connect([&](std::string h, uint64_t) { answer(h, "played its file to the end", true); });
        cl->serverGpioOutStoppedSDMissingNofiy.connect([&](std::string h) { answer(h, "stopped: its file (gpio_out_file_sd) is missing", false); });
        cl->serverGpioOutStoppedSDBrokenNofiy.connect([&](std::string h) { answer(h, "stopped: its file is broken", false); });
        cl->serverGpioOutStoppedSDEmptyNofiy.connect([&](std::string h) { answer(h, "stopped: its file is empty", false); });
        cl->serverGpioOutStoppedMemErrorNofiy.connect([&](std::string h) { answer(h, "stopped: not enough memory (gpio_out_size)", false); });
        cl->serverGpioOutStoppedConfigErrorNofiy.connect([&](std::string h) { answer(h, "did not start: the settings, no GPIO block or the data port taken (journalctl -u redpitaya_streaming on the board)", false); });
    } else {
        cl->serverGpioInStartedNofiy.connect([&](std::string h) { answer(h, "started, it waits for a client to take the data", start); });
        cl->serverGpioInStartedSDNofiy.connect([&](std::string h) { answer(h, "started, it writes a file in streaming_files/gpio", start); });
        cl->serverGpioInStoppedNofiy.connect([&](std::string h) { answer(h, "stopped", !start); });
        cl->serverGpioInStoppedSDDoneNofiy.connect([&](std::string h) { answer(h, "stopped: its file has the samples of gpio_in_samples_limit_sd", true); });
        cl->serverGpioInStoppedSDFullNofiy.connect([&](std::string h) { answer(h, "stopped: the SD card is full", false); });
        cl->serverGpioInStoppedSDBrokenNofiy.connect([&](std::string h) { answer(h, "stopped: the file cannot be written", false); });
        cl->serverGpioInStoppedMemErrorNofiy.connect([&](std::string h) { answer(h, "stopped: not enough memory (gpio_in_size)", false); });
        cl->serverGpioInStoppedConfigErrorNofiy.connect([&](std::string h) { answer(h, "did not start: the settings, no GPIO block or the data port taken (journalctl -u redpitaya_streaming on the board)", false); });
    }
    count = static_cast<int>(hosts.size());
    for (auto& host : hosts) {
        bool sent = start ? (output ? cl->sendGPIOOutServerStart(host) : cl->sendGPIOInServerStart(host))
                          : (output ? cl->sendGPIOOutServerStop(host) : cl->sendGPIOInServerStop(host));
        if (!sent) {
            aprintf(stderr, "%s [ERROR] %s: the command could not be sent\n", getTS(": ").c_str(), host.c_str());
            failed = true;
            count--;
        }
    }
    const std::string what = std::string(start ? "the start" : "the stop") + " of the GPIO " + dir;
    const bool ok = waitAnswers(count, [] { return g_rexit_flag.load(); }, what.c_str());
    cl->removeHadlers();
    return ok && !failed;
}

auto startRemote(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option, std::map<std::string, dac_channels_t>* activeChannels,
                 std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool {
    std::list<std::string> connected_hosts;
    std::list<std::string> slaveHosts;
    std::list<std::string> masterHosts;
    g_rexit_flag = false;
    g_roption = option;

    connected_hosts = cl->getHosts();
    for (auto& host : connected_hosts) {
        switch (cl->getModeByHost(host)) {
            case broadcast_lib::AB_SERVER_MASTER:
                masterHosts.push_back(host);
                break;
            case broadcast_lib::AB_SERVER_SLAVE:
                slaveHosts.push_back(host);
                break;
            default:
                break;
        }
    }
    // The handlers of a refusal take the board off the lists. -r alone (no runned_hosts) fails then:
    // exit code 3. The callers with runned_hosts (-s, -o) go on with the boards that started.
    const size_t boards = masterHosts.size() + slaveHosts.size();
    auto allStarted = [&](bool ok) { return ok && (runned_hosts || masterHosts.size() + slaveHosts.size() == boards); };
    switch (g_roption.remote_mode) {
        case ClientOpt::RemoteMode::START: {
            return allStarted(startStreaming(cl, masterHosts, slaveHosts, runned_hosts));
        }

        case ClientOpt::RemoteMode::START_WITH_ADC: {
            if (allStarted(startStreaming(cl, masterHosts, slaveHosts, runned_hosts))) {
                return allStarted(startADC(cl, masterHosts, slaveHosts, runned_hosts));
            }
            return false;
        }

        case ClientOpt::RemoteMode::START_DAC: {
            return startDACStreaming(cl, masterHosts, slaveHosts, activeChannels, runned_hosts);
        }

        case ClientOpt::RemoteMode::START_FPGA_ADC: {
            return allStarted(startADC(cl, masterHosts, slaveHosts, runned_hosts));
        }

        case ClientOpt::RemoteMode::STOP: {
            return stopStreaming(cl, masterHosts, slaveHosts, runned_hosts);
        }

        case ClientOpt::RemoteMode::STOP_DAC: {
            return stopDACStreaming(cl, masterHosts, slaveHosts, runned_hosts);
        }

        case ClientOpt::RemoteMode::START_STOP: {
            return startStopStreaming(cl, masterHosts, slaveHosts, runned_hosts);
        }

        case ClientOpt::RemoteMode::START_STOP_DAC: {
            return startStopDACStreaming(cl, masterHosts, slaveHosts, activeChannels, runned_hosts);
        }

        case ClientOpt::RemoteMode::START_GPIO_IN:
        case ClientOpt::RemoteMode::STOP_GPIO_IN:
        case ClientOpt::RemoteMode::START_GPIO_OUT:
        case ClientOpt::RemoteMode::STOP_GPIO_OUT: {
            const auto mode = g_roption.remote_mode;
            return remoteGpio(cl, connected_hosts, mode == ClientOpt::RemoteMode::START_GPIO_IN || mode == ClientOpt::RemoteMode::START_GPIO_OUT,
                              mode == ClientOpt::RemoteMode::START_GPIO_OUT || mode == ClientOpt::RemoteMode::STOP_GPIO_OUT);
        }

        case ClientOpt::RemoteMode::WATCH:
        case ClientOpt::RemoteMode::STATUS: {
            return watchRecords(cl, connected_hosts, g_roption, g_roption.remote_mode == ClientOpt::RemoteMode::STATUS, [] { return g_rexit_flag.load(); });
        }

        default: {
            aprintf(stderr, "%s [Fatal] Error mode\n", getTS(": ").c_str());
            return false;
        }
    }
    return true;
}

auto remoteSIGHandler() -> void {
    g_rexit_flag = true;
}

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                    std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool {
    std::atomic<int> rstart_counter;

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
        const std::lock_guard lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            rstart_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->configFileMissedNotify.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: its config file is missing\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStoppedMemErrorNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: not enough DMA memory (adc_size)\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStoppedMemModifyNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: the memory of the streams was split again (a new block size)\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStoppedNoActiveChannelsNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: no input is on (channel_state_N=ON)\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStoppedConfigErrorNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        // always: the reason is not seen anywhere else (the log of the board)
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: the settings or the board (journalctl -u redpitaya_streaming on the board)\n", getTS(": ").c_str(),
                host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStartedTCPNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s Streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        if (runned_hosts)
            (*runned_hosts)[host] = StateRunnedHosts::TCP;
    });

    cl->serverStartedSDNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s Streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        if (runned_hosts)
            (*runned_hosts)[host] = StateRunnedHosts::LOCAL;
    });

    rstart_counter = slaveHosts.size();
    for (auto& host : slaveHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send start command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendADCServerStart(host)) {
            rstart_counter--;
        }
    }
    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the start of the streaming")) {
        cl->removeHadlers();
        return false;
    }

    rstart_counter = masterHosts.size();
    for (auto& host : masterHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send start command to master board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendADCServerStart(host)) {
            rstart_counter--;
        }
    }

    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the start of the streaming")) {
        cl->removeHadlers();
        return false;
    }
    cl->removeHadlers();
    return true;
}

auto requestMemoryBlockSize(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& hosts, std::map<std::string, uint32_t>* sizes, bool verbose) -> bool {
    std::atomic<int> rstart_counter;

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            rstart_counter--;
        }
    });

    cl->getMemBlockSizeNofiy.connect([&](std::string host, std::string size) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (verbose)
            aprintf(stdout, "%s Memory block size of %s is %s\n", getTS(": ").c_str(), host.c_str(), size.c_str());
        rstart_counter--;
        if (sizes)
            (*sizes)[host] = atoi(size.c_str());
    });

    rstart_counter = hosts.size();
    for (auto& host : hosts) {
        if (verbose)
            aprintf(stdout, "%s Request for memory block size sent : %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestMemoryBlockSize(host)) {
            rstart_counter--;
        }
    }
    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the request of the memory block size")) {
        cl->removeHadlers();
        return false;
    }
    cl->removeHadlers();
    return true;
}

auto requestActiveChannels(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& hosts, std::map<std::string, adc_channels_t>* channels, bool verbose) -> bool {
    std::atomic<int> rstart_counter;

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            rstart_counter--;
        }
    });

    cl->getActiveChannelsNofiy.connect([&](std::string host, adc_channels_t ch) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (verbose)
            aprintf(stdout, "%s %s Active channels: %s\n", getTS(": ").c_str(), host.c_str(), ch.format().c_str());
        rstart_counter--;
        if (channels)
            (*channels)[host] = ch;
    });

    rstart_counter = hosts.size();
    for (auto& host : hosts) {
        if (verbose)
            aprintf(stdout, "%s Request for active channels sent : %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->requestActiveChannels(host)) {
            rstart_counter--;
        }
    }
    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the request of the active channels")) {
        cl->removeHadlers();
        return false;
    }
    cl->removeHadlers();
    return true;
}

auto startADC(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
              std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool {
    std::atomic<int> rstart_counter;

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
        const std::lock_guard lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            rstart_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->serverStoppedMemErrorNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: not enough DMA memory (adc_size)\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStoppedMemModifyNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: the memory of the streams was split again (a new block size)\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStoppedNoActiveChannelsNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: no input is on (channel_state_N=ON)\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverStoppedConfigErrorNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        // always: the reason is not seen anywhere else (the log of the board)
        aprintf(stderr, "%s [ERROR] %s did not start the ADC: the settings or the board (journalctl -u redpitaya_streaming on the board)\n", getTS(": ").c_str(),
                host.c_str());
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->startADCDoneNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s ADC is run: %s\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        if (runned_hosts)
            (*runned_hosts)[host] = StateRunnedHosts::NONE;
    });

    rstart_counter = slaveHosts.size();

    for (auto& host : slaveHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send start ADC command to slave board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendADCFPGAStart(host)) {
            rstart_counter--;
        }
    }
    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the start of the ADC")) {
        cl->removeHadlers();
        return false;
    }

    rstart_counter = masterHosts.size();
    for (auto& host : masterHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send start ADC command to master board: %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendADCFPGAStart(host)) {
            rstart_counter--;
        }
    }
    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the start of the ADC")) {
        cl->removeHadlers();
        return false;
    }
    cl->removeHadlers();
    return true;
}

// The end of a file played from the SD card can come at any time after the start: the host leaves
// the lists, so a start-stop does not wait for the timeout and sends it no stop
auto connectDACFileEnd(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts) -> void {
    auto ended = [&masterHosts, &slaveHosts](std::string host, const char* what) {
        const std::lock_guard lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming stopped: %s %s [OK]\n", getTS(": ").c_str(), host.c_str(), what);
        masterHosts.remove(host);
        slaveHosts.remove(host);
    };
    cl->serverDacStoppedSDDoneNofiy.connect([ended](std::string host) { ended(host, "SD done"); });
    cl->serverDacStoppedSDBrokenNofiy.connect([ended](std::string host) { ended(host, "SD File broken"); });
    cl->serverDacStoppedSDEmptyNofiy.connect([ended](std::string host) { ended(host, "SD File empty"); });
    cl->serverDacStoppedSDMissingNofiy.connect([ended](std::string host) { ended(host, "SD File missing"); });
}

auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                       std::map<std::string, dac_channels_t>* activeChannels, std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool {
    connectDACFileEnd(cl, masterHosts, slaveHosts);
    std::atomic<int> rstart_counter;
    // a board that refused: -r alone (no runned_hosts) exits with 3 then
    std::atomic<bool> refused{false};

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
        const std::lock_guard lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            refused = true;
            rstart_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    // always: the reasons are not seen anywhere else (the log of the board)
    cl->serverDacStoppedMemErrorNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the DAC: not enough memory (dac_size) or no active channels\n", getTS(": ").c_str(), host.c_str());
        refused = true;
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverDacStoppedMemModifyNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the DAC: the memory of the streams was split again (a new block size)\n", getTS(": ").c_str(), host.c_str());
        refused = true;
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverDacStoppedConfigErrorNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the DAC: the settings or the board (journalctl -u redpitaya_streaming on the board)\n", getTS(": ").c_str(),
                host.c_str());
        refused = true;
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->configFileMissedNotify.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr, "%s [ERROR] %s did not start the DAC: its config file is missing\n", getTS(": ").c_str(), host.c_str());
        refused = true;
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    });

    cl->serverDacStartedNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming started: %s TCP mode [OK]\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        if (runned_hosts)
            (*runned_hosts)[host] = StateRunnedHosts::TCP;
    });

    cl->serverDacStartedSDNofiy.connect([&](std::string host) {
        const std::lock_guard lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming started: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
        rstart_counter--;
        if (runned_hosts)
            (*runned_hosts)[host] = StateRunnedHosts::LOCAL;
    });

    // the board plays a file of its own SD card (dac_pass_mode=DAC_FILE) and cannot: the start
    // ends at once
    auto ownFile = [&](const std::string& host, const char* what) {
        const std::lock_guard lock(g_rmutex);
        aprintf(stderr,
                "%s [ERROR] %s plays a file of its own SD card (dac_pass_mode=DAC_FILE), and %s.\n"
                "        To play from here: rpsa_client -c -h %s -i dac_pass_mode=DAC_NET\n",
                getTS(": ").c_str(),
                host.c_str(),
                what,
                host.c_str());
        refused = true;
        rstart_counter--;
        masterHosts.remove(host);
        slaveHosts.remove(host);
    };
    cl->serverDacStoppedSDMissingNofiy.connect([&](std::string host) { ownFile(host, "that file (dac_file_sd) is missing"); });
    cl->serverDacStoppedSDBrokenNofiy.connect([&](std::string host) { ownFile(host, "that file is broken"); });
    cl->serverDacStoppedSDEmptyNofiy.connect([&](std::string host) { ownFile(host, "that file is empty"); });

    rstart_counter = slaveHosts.size();

    for (auto host : std::list<std::string>(slaveHosts)) {
        dac_channels_t ac;
        if (activeChannels != nullptr) {
            if (activeChannels->count(host)) {
                ac = activeChannels->at(host);
            }
        }
        if (g_roption.verbous)
            aprintf(stdout, "%s Send start DAC command to slave board: %s Active channels: %s\n", getTS(": ").c_str(), host.c_str(), ac.format().c_str());
        if (!cl->sendDACServerStart(host, ac)) {
            rstart_counter--;
        }
    }
    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the start of the DAC streaming")) {
        cl->removeHadlers();
        return false;
    }

    rstart_counter = masterHosts.size();
    // a copy: a host leaves the list when its file ends
    for (auto host : std::list<std::string>(masterHosts)) {
        dac_channels_t ac;
        if (activeChannels != nullptr) {
            if (activeChannels->count(host)) {
                ac = activeChannels->at(host);
            }
        }
        if (g_roption.verbous)
            aprintf(stdout, "%s Send start DAC command to master board: %s Active channels: %s\n", getTS(": ").c_str(), host.c_str(), ac.format().c_str());
        if (!cl->sendDACServerStart(host, ac)) {
            rstart_counter--;
        }
    }
    if (!waitAnswers(rstart_counter, [] { return g_rexit_flag.load(); }, "the start of the DAC streaming")) {
        cl->removeHadlers();
        return false;
    }
    cl->removeHadlers();
    return runned_hosts || !refused;
}

auto stopStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                   std::map<std::string, StateRunnedHosts>*) -> bool {
    std::atomic<int> rstop_counter;

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            rstop_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->serverStoppedNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->serverStoppedSDFullNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s Streaming stopped: %s SD card is full [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->serverStoppedSDDoneNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s Streaming stopped: %s All samples are recorded on the SD card [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    rstop_counter = masterHosts.size();
    for (auto& host : masterHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send stop command to master board %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendADCServerStop(host)) {
            rstop_counter--;
        }
    }
    if (!waitAnswers(rstop_counter, [] { return g_rexit_flag.load(); }, "the stop of the streaming")) {
        cl->removeHadlers();
        return false;
    }

    rstop_counter = slaveHosts.size();
    for (auto& host : slaveHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send stop command to slave board %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendADCServerStop(host)) {
            rstop_counter--;
        }
    }
    if (!waitAnswers(rstop_counter, [] { return g_rexit_flag.load(); }, "the stop of the streaming")) {
        cl->removeHadlers();
        return false;
    }
    cl->removeHadlers();
    return true;
}

auto stopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                      std::map<std::string, StateRunnedHosts>*) -> bool {
    std::atomic<int> rstop_counter;

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
            rstop_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->serverDacStoppedNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->serverDacStoppedSDDoneNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming stopped: %s SD done [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->serverDacStoppedSDBrokenNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming stopped: %s SD File broken [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->serverDacStoppedSDEmptyNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming stopped: %s SD File empty [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->serverDacStoppedSDMissingNofiy.connect([&](std::string host) {
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            aprintf(stdout, "%s DAC streaming stopped: %s SD File missing [OK]\n", getTS(": ").c_str(), host.c_str());
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    rstop_counter = masterHosts.size();
    for (auto& host : masterHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send DAC stop command to master board %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendDACServerStop(host)) {
            rstop_counter--;
        }
    }
    if (!waitAnswers(rstop_counter, [] { return g_rexit_flag.load(); }, "the stop of the DAC streaming")) {
        cl->removeHadlers();
        return false;
    }

    rstop_counter = slaveHosts.size();
    for (auto& host : slaveHosts) {
        if (g_roption.verbous)
            aprintf(stdout, "%s Send DAC stop command to slave board %s\n", getTS(": ").c_str(), host.c_str());
        if (!cl->sendDACServerStop(host)) {
            rstop_counter--;
        }
    }
    if (!waitAnswers(rstop_counter, [] { return g_rexit_flag.load(); }, "the stop of the DAC streaming")) {
        cl->removeHadlers();
        return false;
    }
    cl->removeHadlers();
    return true;
}

auto startStopStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                        std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool {
    if (!startStreaming(cl, masterHosts, slaveHosts, runned_hosts)) {
        return false;
    }

    if (!startADC(cl, masterHosts, slaveHosts, runned_hosts)) {
        return false;
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    g_roption.timeout = g_roption.timeout == 0 ? 1000 : g_roption.timeout;
    if (!runned_hosts) {
        // -r start_stop: the boards writing their SD cards are shown while it waits (it ends at the
        // timeout or when they are done; then the wait below goes on)
        std::list<std::string> hosts(masterHosts.begin(), masterHosts.end());
        hosts.insert(hosts.end(), slaveHosts.begin(), slaveHosts.end());
        watchRecords(cl, hosts, g_roption, false, [&] {
            const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
            return g_rexit_flag.load() || now - beginTime > g_roption.timeout;
        }, false);
    }
    while ((curTime - beginTime <= g_roption.timeout) && (masterHosts.size() > 0 || slaveHosts.size() > 0)) {
        sleepMs(1);
        if (g_rexit_flag)
            break;
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }

    g_rexit_flag = false;

    if (!stopStreaming(cl, masterHosts, slaveHosts, runned_hosts)) {
        return false;
    }

    return true;
}

auto startStopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, std::list<std::string>& masterHosts, std::list<std::string>& slaveHosts,
                           std::map<std::string, dac_channels_t>* activeChannels, std::map<std::string, StateRunnedHosts>* runned_hosts) -> bool {
    if (!startDACStreaming(cl, masterHosts, slaveHosts, activeChannels, runned_hosts)) {
        return false;
    }

    connectDACFileEnd(cl, masterHosts, slaveHosts);
    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    g_roption.timeout = g_roption.timeout == 0 ? 1000 : g_roption.timeout;
    while ((curTime - beginTime <= g_roption.timeout) && (masterHosts.size() > 0 || slaveHosts.size() > 0)) {
        sleepMs(1);
        if (g_rexit_flag)
            break;
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }
    cl->removeHadlers();

    if (!stopDACStreaming(cl, masterHosts, slaveHosts, runned_hosts)) {
        return false;
    }

    return true;
}
