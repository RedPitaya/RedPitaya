#include <filesystem>
#include "panel.h"
#include "test_helper.h"
#include "config.h"
#include <chrono>

#include "dac_streaming.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "logger_lib/file_logger.h"
#include "remote.h"
#include "settings_lib/dac_settings_client.h"

using namespace dac_streaming_lib;

std::map<std::string, CDACStreamingManager::Ptr> g_dac_manger;
std::map<std::string, net_lib::CAsioNet::Ptr> g_dac_asionet;

std::mutex g_dac_smutex;
ClientOpt::Options g_dac_soption;
std::atomic<bool> dac_sig_exit_flag(false);
std::atomic<bool> g_dacFailed(false);  // the file could not be played or sent
std::atomic<int> g_dacRunning(0);
std::map<std::string, uint64_t> g_dac_bytesTotal;  // sent per board
std::map<std::string, bool> g_dac_lost;  // the data connection broke before the file was sent
long long g_dac_start = 0;

std::map<std::string, long long int> g_dac_timeBegin;
std::map<std::string, bool> g_dac_terminate;
std::map<std::string, bool> g_dac_connected;
std::map<std::string, bool> g_dac_fileEnded;
std::map<std::string, uint64_t> g_dac_BytesCount;
std::map<std::string, uint64_t> g_dac_packCounter_ch1;
std::map<std::string, uint64_t> g_dac_packCounter_ch2;

std::vector<std::thread> dac_clients;

auto stopDACStreaming() -> void;
auto stopDACStreaming(std::string host) -> void;

auto getActiveChannels(DACSettingsClient conf) -> dac_channels_t {
    auto file_type = CDACStreamingManager::WAV_TYPE;

    switch (conf.file_type) {
        case CStreamSettings::DataFormat::TDMS:
            file_type = CDACStreamingManager::TDMS_TYPE;
            break;
        case CStreamSettings::DataFormat::WAV:
            file_type = CDACStreamingManager::WAV_TYPE;
            break;
        case CStreamSettings::DataFormat::BIN:
            file_type = CDACStreamingManager::BIN_TYPE;
            break;
        default:
            return {};
    }
    dac_channels_t ac;
    auto x = CDACStreamingManager::Create(file_type, conf.dac_file, CStreamSettings::DACRepeat::DAC_REP_OFF, 0, 0, false);
    x->getChannels(ac);
    return ac;
}

auto dac_runClient(DACSettingsClient conf, uint32_t blockSize) -> void {
    auto memoryManager = new uio_lib::CMemoryManager();
    memoryManager->setMemoryBlockSize(blockSize);
    memoryManager->reallocateBlocks();
    auto blocks = memoryManager->getFreeBlockCount();

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    g_dac_timeBegin[conf.host] = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow).time_since_epoch().count();

    g_dac_packCounter_ch1[conf.host] = 0;
    g_dac_packCounter_ch2[conf.host] = 0;
    g_dac_BytesCount[conf.host] = 0;
    g_dac_terminate[conf.host] = false;
    g_dac_fileEnded[conf.host] = false;

    auto file_type = CDACStreamingManager::WAV_TYPE;

    switch (conf.file_type) {
        case CStreamSettings::DataFormat::TDMS:
            file_type = CDACStreamingManager::TDMS_TYPE;
            break;
        case CStreamSettings::DataFormat::WAV:
            file_type = CDACStreamingManager::WAV_TYPE;
            break;
        case CStreamSettings::DataFormat::BIN:
            file_type = CDACStreamingManager::BIN_TYPE;
            break;
        default:
            stopDACStreaming(conf.host);
            return;
    }

    auto rep_mode = CStreamSettings::DACRepeat::DAC_REP_OFF;

    if (conf.dac_repeat == (int)ClientOpt::RepeatDAC::INF) {
        rep_mode = CStreamSettings::DACRepeat::DAC_REP_INF;
    }
    if (conf.dac_repeat >= 0) {
        rep_mode = CStreamSettings::DACRepeat::DAC_REP_ON;
    }

    int dac_rep = conf.dac_repeat >= 0 ? conf.dac_repeat : 0;

    g_dac_manger[conf.host] = CDACStreamingManager::Create(file_type, conf.dac_file, rep_mode, dac_rep, blockSize, conf.verbous);
    g_dac_manger[conf.host]->setRemoteClientMode(true);

    dac_channels_t chActive;
    if (!g_dac_manger[conf.host]->getChannels(chActive)) {
        FATAL("Incorrect mode")
    }

    auto reserved __attribute__((unused)) = memoryManager->reserveMemory(uio_lib::MM_DAC, blocks, chActive.count());
    g_dac_manger[conf.host]->getBufferManager()->generateBuffersEmptyDAC(chActive, memoryManager->getRegions(uio_lib::MM_DAC), DataLib::sizeHeader());
    g_dac_manger[conf.host]->getBufferManager()->initHeadersDAC(chActive);
    TRACE_SHORT("Reserved blocks %d", reserved)

    g_dac_manger[conf.host]->notifyStop.connect([=](CDACStreamingManager::NotifyResult res) {
        const std::lock_guard lock(g_dac_smutex);
        switch (res) {
            case CDACStreamingManager::NotifyResult::NR_BROKEN: {
                g_dacFailed = true;
                aprintf(stderr, "%s [ERROR] File %s is broken\n", getTS(": ").c_str(), conf.dac_file.c_str());
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_EMPTY: {
                g_dacFailed = true;
                aprintf(stderr, "%s [ERROR] File %s is empty\n", getTS(": ").c_str(), conf.dac_file.c_str());
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_MISSING_FILE: {
                g_dacFailed = true;
                aprintf(stderr, "%s [ERROR] File %s is missing\n", getTS(": ").c_str(), conf.dac_file.c_str());
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_STOP: {
                if (conf.verbous)
                    aprintf(stdout, "%s Got stop command from data controller\n", getTS(": ").c_str());
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_ENDED: {
                if (conf.verbous)
                    aprintf(stdout, "%s All data from the file has been read: %s\n", getTS(": ").c_str(), conf.dac_file.c_str());
                g_dac_fileEnded[conf.host] = true;
                break;
            }
            default:
                break;
        }
        g_dac_terminate[conf.host] = true;
    });

    g_dac_connected[conf.host] = false;
    g_dac_asionet[conf.host] = net_lib::CAsioNet::create(net_lib::EMode::M_CLIENT, conf.host, NET_DAC_STREAMING_PORT, nullptr);
    g_dac_asionet[conf.host]->clientConnectNotify.connect([=](std::string host) {
        const std::lock_guard lock(g_dac_smutex);
        if (conf.verbous)
            aprintf(stdout, "%s CLIENT CONNECTED  %s\n", getTS(": ").c_str(), host.c_str());
        g_dac_connected[host] = true;
    });

    g_dac_asionet[conf.host]->clientDisconnectNotify.connect([=](std::string host) {
        const std::lock_guard lock(g_dac_smutex);
        if (g_dac_connected[host] && conf.verbous)
            aprintf(stdout, "%s CLIENT DISCONNECTED  %s\n", getTS(": ").c_str(), host.c_str());
        g_dac_connected[host] = false;
    });

    g_dac_asionet[conf.host]->sendNotify.connect([=](std::error_code err, size_t) {
        if (err.value() != 0 && g_dac_terminate[conf.host] == false) {
            if (conf.verbous)
                aprintf(stdout, "%s SEND ERROR  %s\n", getTS(": ").c_str(), conf.host.c_str());
            g_dacFailed = true;
            g_dac_lost[conf.host] = true;
            g_dac_terminate[conf.host] = true;
        }
    });

    g_dac_asionet[conf.host]->start();

    const auto connectEnd = std::chrono::steady_clock::now() + std::chrono::milliseconds(replyTimeout());
    while (!g_dac_connected[conf.host] && !g_dac_terminate[conf.host] && std::chrono::steady_clock::now() < connectEnd)
        sleepMs(10);
    if (!g_dac_connected[conf.host]) {
        aprintf(stderr, "%s [ERROR] %s: no data connection for the DAC within %.1f s\n", getTS(": ").c_str(), conf.host.c_str(), replyTimeout() / 1000.0);
        g_dacFailed = true;
    }

    if (g_dac_connected[conf.host]) {
        g_dac_manger[conf.host]->run();
        while (1) {
            auto pack = g_dac_manger[conf.host]->getBuffer();
            if (pack) {
                g_dac_asionet[conf.host]->sendSyncData(pack);
                auto ch1 = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
                auto ch2 = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH2);

                const std::lock_guard lock(g_dac_smutex);
                if (ch1) {
                    g_dac_packCounter_ch1[conf.host]++;
                    g_dac_BytesCount[conf.host] += ch1->getDataLenght();
                    g_dac_bytesTotal[conf.host] += ch1->getDataLenght();
                }

                if (ch2) {
                    g_dac_packCounter_ch2[conf.host]++;
                    g_dac_BytesCount[conf.host] += ch2->getDataLenght();
                    g_dac_bytesTotal[conf.host] += ch2->getDataLenght();
                }
                g_dac_manger[conf.host]->unlockBuffer();
            }

            if (g_dac_terminate[conf.host]) {
                if (g_dac_connected[conf.host]) {
                    if (g_dac_fileEnded[conf.host]) {
                        if (g_dac_manger[conf.host]->isEmptyBuffer())
                            break;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }

            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
            auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
            auto value = curTime.time_since_epoch();
            if (conf.verbous && (value.count() - g_dac_timeBegin[conf.host]) >= 5000) {
                const std::lock_guard lock(g_dac_smutex);
                uint64_t bw = g_dac_BytesCount[conf.host];
                std::string pref = " ";
                if (g_dac_BytesCount[conf.host] > (1024 * 5)) {
                    bw = g_dac_BytesCount[conf.host] / (1024 * 5);
                    pref = " ki";
                }

                if (g_dac_BytesCount[conf.host] > (1024 * 1024 * 5)) {
                    bw = g_dac_BytesCount[conf.host] / (1024 * 1024 * 5);
                    pref = " Mi";
                }
                aprintf(stdout,
                        "%s\tHOST IP: %s: Bandwidth:\t%d %sB/s \tData count ch1:\t%d\tch2:\t%d\n",
                        getTS(": ").c_str(),
                        conf.host.c_str(),
                        bw,
                        pref.c_str(),
                        g_dac_packCounter_ch1[conf.host],
                        g_dac_packCounter_ch2[conf.host]);
                g_dac_BytesCount[conf.host] = 0;
                g_dac_timeBegin[conf.host] = value.count();
            }
        }
    }
    stopDACStreaming(conf.host);
    delete memoryManager;
    g_dacRunning--;
}

auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool {
    g_dac_soption = option;
    g_dacFailed = false;
    g_dac_bytesTotal.clear();
    g_dac_lost.clear();

    std::map<std::string, uint32_t> blockSizes;
    auto hosts = cl->getHosts();
    if (!requestMemoryBlockSize(cl, hosts, &blockSizes, option.verbous)) {
        aprintf(stdout, "%s Can't get block sizes\n", getTS(": ").c_str());
        return false;
    }

    std::map<std::string, dac_channels_t> acMap;
    for (auto& item : cl->getHosts()) {
        DACSettingsClient conf;
        conf.host = item;
        switch (g_dac_soption.streamign_type) {
            case ClientOpt::StreamingType::TDMS:
                conf.file_type = CStreamSettings::DataFormat::TDMS;
                break;
            case ClientOpt::StreamingType::WAV:
                conf.file_type = CStreamSettings::DataFormat::WAV;
                break;
            case ClientOpt::StreamingType::BIN:
                conf.file_type = CStreamSettings::DataFormat::BIN;
                break;
            default:
                return false;
        }
        conf.dac_file = g_dac_soption.dac_file;
        conf.dac_repeat = 0;
        conf.verbous = false;
        auto ac = getActiveChannels(conf);
        acMap[item] = ac;
    }

    ClientOpt::Options remote_opt = g_dac_soption;
    remote_opt.mode = ClientOpt::Mode::REMOTE;
    remote_opt.remote_mode = ClientOpt::RemoteMode::START_DAC;
    remote_opt.verbous = g_dac_soption.verbous;
    std::map<std::string, StateRunnedHosts> runned_hosts;
    if (startRemote(cl, remote_opt, &acMap, &runned_hosts)) {
        // a board that plays its own file takes nothing from here
        bool local = false;
        for (auto& kv : runned_hosts) {
            if (kv.second == StateRunnedHosts::LOCAL) {
                aprintf(stderr,
                        "%s [ERROR] %s plays a file of its own SD card (dac_pass_mode=DAC_FILE), not from this client.\n"
                        "        To play from here: rpsa_client -c -h %s -i dac_pass_mode=DAC_NET\n",
                        getTS(": ").c_str(),
                        kv.first.c_str(),
                        kv.first.c_str());
                local = true;
            }
        }
        if (local) {
            remote_opt.remote_mode = ClientOpt::RemoteMode::STOP_DAC;
            startRemote(cl, remote_opt, &acMap, &runned_hosts);
            return false;
        }
        if (runned_hosts.empty()) {
            aprintf(stderr, "%s [ERROR] No board started the DAC streaming\n", getTS(": ").c_str());
            return false;
        }
        for (auto& kv : runned_hosts) {
            if (kv.second == StateRunnedHosts::TCP) {
                DACSettingsClient conf;
                conf.host = kv.first;
                switch (g_dac_soption.streamign_type) {
                    case ClientOpt::StreamingType::TDMS:
                        conf.file_type = CStreamSettings::DataFormat::TDMS;
                        break;
                    case ClientOpt::StreamingType::WAV:
                        conf.file_type = CStreamSettings::DataFormat::WAV;
                        break;
                    case ClientOpt::StreamingType::BIN:
                        conf.file_type = CStreamSettings::DataFormat::BIN;
                        break;
                    default:
                        return false;
                }
                conf.dac_file = g_dac_soption.dac_file;
                conf.dac_repeat = g_dac_soption.dac_repeat;
                conf.verbous = g_dac_soption.verbous;
                g_dacRunning++;
                dac_clients.push_back(std::thread(dac_runClient, conf, blockSizes[kv.first]));
            }
        }

        // the panel on a terminal while the file plays
        auto nowMs = [] { return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count(); };
        g_dac_start = nowMs();
        {
            static const char* formats[] = {"", "TDMS", "WAV", "CSV", "BIN"};
            std::error_code ec;
            const uint64_t fileBytes = std::filesystem::file_size(g_dac_soption.dac_file, ec);
            const bool inf = g_dac_soption.dac_repeat == (int)ClientOpt::RepeatDAC::INF;
            const std::string repeats = inf ? "inf" : std::to_string(g_dac_soption.dac_repeat);
            Panel panel(g_dac_soption);
            while (panel.enabled() && g_dacRunning > 0) {
                const double secs = (nowMs() - g_dac_start) / 1000.0;
                char buf[256];
                std::vector<std::string> lines;
                snprintf(buf, sizeof(buf), "  %s (%s, %s)   repeat %s   %.1f s", g_dac_soption.dac_file.c_str(), formats[(int)g_dac_soption.streamign_type],
                         sizeText(fileBytes).c_str(), repeats.c_str(), secs);
                lines.push_back(Panel::bold("rpsa_client -o") + buf);
                lines.push_back("");
                lines.push_back(Panel::dim(Panel::left("  board", 16) + Panel::left("state", 11) + Panel::right("pass", 7) + "  " +
                                           Panel::left("in the file", 19) + Panel::right("sent", 10) + Panel::right("rate", 11)));
                {
                    const std::lock_guard lock(g_dac_smutex);
                    for (auto& [host, bytes] : g_dac_bytesTotal) {
                        // the file is the data plus its header: the pass and the position are close, not exact
                        const uint64_t pass = fileBytes ? bytes / fileBytes : 0;
                        const double pos = fileBytes ? (double)(bytes % fileBytes) / fileBytes : 0;
                        const std::string state = g_dac_lost[host] ? Panel::red("stopped") : g_dac_connected[host] ? Panel::green("playing") : std::string("connecting");
                        snprintf(buf, sizeof(buf), "%3.0f %%", pos * 100);
                        lines.push_back(Panel::left("  " + host, 16) + Panel::left(state, 11) +
                                        Panel::right(std::to_string(pass + 1) + "/" + repeats, 7) + "  " + Panel::left(Panel::bar(pos, 10) + " " + buf, 19) +
                                        Panel::right(sizeText(bytes), 10) + Panel::right(sizeText(secs > 0 ? (uint64_t)(bytes / secs) : 0) + "/s", 11));
                    }
                    if (g_dac_bytesTotal.empty())
                        lines.push_back("  connecting...");
                }
                lines.push_back("");
                lines.push_back(Panel::dim(std::string("  ") + (panel.keys() ? "[q] stop   " : "") + "[Ctrl+C] stop"));
                panel.draw(lines);
                const int k = panel.key();
                if (k == 'q' || k == 'Q') {
                    dac_sig_exit_flag = true;
                    stopDACStreaming();
                }
                sleepMs(250);
            }
        }
        for (std::thread& t : dac_clients) {
            if (t.joinable()) {
                t.join();
            }
        }
        dac_clients.clear();
        if (!g_dac_lost.empty())
            sleepMs(300);  // the closed config connection is seen
        for (auto& [host, lost] : g_dac_lost) {
            if (lost)
                aprintf(stderr, "%s [ERROR] %s ended the DAC streaming early: %s\n", getTS(": ").c_str(), host.c_str(), endedText(cl->isConnected(host)));
        }
        const double secs = (nowMs() - g_dac_start) / 1000.0;
        const bool repeats = g_dac_soption.dac_repeat == (int)ClientOpt::RepeatDAC::INF;
        for (auto& [host, bytes] : g_dac_bytesTotal) {
            const std::string hint = repeats && !g_dac_lost[host] ? "; the board repeats it until: rpsa_client -r -h " + host + " -m stop_dac" : "";
            aprintf(stdout, "%s %s: sent %s of %s in %.1f s%s\n", g_dac_lost[host] ? "Stopped:" : "Done:", host.c_str(), sizeText(bytes).c_str(),
                    g_dac_soption.dac_file.c_str(), secs, hint.c_str());
        }
    } else {
        aprintf(stderr, "%s [ERROR] Can't start the DAC streaming on the boards\n", getTS(": ").c_str());
        g_dacFailed = true;
    }
    return !g_dacFailed;
}

auto dac_streamingSIGHandler() -> void {
    stopDACStreaming();
    dac_sig_exit_flag = true;
}

auto stopDACStreaming(std::string host) -> void {
    if (g_dac_asionet.count(host)) {
        if (g_dac_asionet[host]) {
            g_dac_asionet[host]->stop();
            g_dac_asionet[host] = nullptr;
        }
    }
    if (g_dac_manger.count(host)) {
        if (g_dac_manger[host]) {
            g_dac_manger[host]->stop();
            g_dac_manger[host] = nullptr;
        }
    }
    g_dac_terminate[host] = true;
}

auto stopDACStreaming() -> void {
    for (auto& kv : g_dac_terminate) {
        kv.second = true;
    }
}
