/**
 * The config channel as the applications use it today.
 *
 * The server side is wired the way streaming-server/src/main.cpp wires it: an ADC start answers
 * "started TCP" or "started SD" by the pass mode (or "no channels"), a DAC start answers by the
 * DAC pass mode, the FPGA start commands are confirmed, a block size change is counted.
 *
 * The client side follows rpsa_client (config.cpp, remote.cpp): one ClientNetConfigManager is
 * kept for a whole session, every step connects its handlers, waits for its answers and drops
 * the handlers with removeHadlers() before the next step.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "config_net_lib/client_net_config_manager.h"
#include "config_net_lib/server_net_config_manager.h"

using State = CStreamSettings::State;

namespace {

auto WaitFor(const std::function<bool()>& done, std::chrono::milliseconds timeout = std::chrono::milliseconds(3000)) -> bool {
    const auto end = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < end) {
        if (done())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return done();
}

auto NextPort() -> uint16_t {
    static std::atomic<uint16_t> port{29600};
    return port++;
}

// The answers streaming-server gives, without the streaming behind them.
struct FakeStreamingServer {
    std::unique_ptr<ServerNetConfigManager> config;
    std::atomic<int> adcStarts{0}, adcStops{0}, dacStarts{0}, dacStops{0}, memChanges{0}, newSettings{0};
    std::mutex mutex;
    dac_channels_t dacChannels;

    FakeStreamingServer(const std::string& file, uint16_t port, broadcast_lib::EMode mode = broadcast_lib::AB_SERVER_MASTER) {
        config = std::make_unique<ServerNetConfigManager>(file, mode, "127.0.0.1", port, 2);
        auto* c = config.get();
        c->getNewSettingsNofiy.connect([this]() { newSettings++; });
        c->memoryBlockSizeChangeNofiy.connect([this]() { memChanges++; });
        c->startStreamingNofiy.connect([this, c]() {
            adcStarts++;
            const auto s = c->getSettings();
            bool any = false;
            for (uint8_t ch = 1; ch <= 2; ch++)
                any |= s.getADCChannels(ch) == State::ON;
            if (!any) {
                c->sendServerNoChannelsStopped();
            } else if (s.getADCPassMode() == CStreamSettings::PassMode::FILE) {
                c->sendADCServerStartedSD();
            } else {
                c->sendADCServerStartedTCP();
            }
        });
        c->stopStreamingNofiy.connect([this, c]() {
            adcStops++;
            c->sendServerStopped();
        });
        c->startDacStreamingNofiy.connect([this, c](dac_channels_t channels) {
            {
                std::lock_guard lock(mutex);
                dacChannels = channels;
            }
            dacStarts++;
            if (c->getSettings().getDACPassMode() == CStreamSettings::DACPassMode::DAC_FILE) {
                c->sendDACServerStartedSD();
            } else {
                c->sendDACServerStarted();
            }
        });
        c->stopDacStreamingNofiy.connect([this, c]() {
            dacStops++;
            c->sendDACServerStopped();
        });
        c->startADCNofiy.connect([c]() { c->sendADCStarted(); });
        c->startDACNofiy.connect([c]() { c->sendDACStarted(); });
    }
};

class AppFlow : public ::testing::Test {
   protected:
    void SetUp() override {
        m_dir = std::filesystem::temp_directory_path() / ("config_net_flow_" + std::to_string(m_port));
        std::filesystem::create_directories(m_dir);
        m_server = std::make_unique<FakeStreamingServer>(serverFile(), m_port);
        m_client = std::make_shared<ClientNetConfigManager>("", false);
    }
    void TearDown() override {
        m_client.reset();
        m_server.reset();
        std::error_code ec;
        std::filesystem::remove_all(m_dir, ec);
    }

    auto serverFile() const -> std::string { return (m_dir / "streaming_config.json").string(); }

    // rpsa_client connect step: wait for every host to answer with its mode.
    auto connect() -> void {
        std::atomic<bool> connected{false};
        m_client->serverConnectedNofiy.connect([&](std::string&) { connected = true; });
        m_client->connectToServers({m_host}, m_port);
        ASSERT_TRUE(WaitFor([&] { return connected.load(); }));
        m_client->removeHadlers();
        ASSERT_EQ(m_client->getHosts(), std::list<std::string>{m_host});
    }

    // One rpsa_client step: send, wait for the answer, drop the handlers.
    auto step(const std::function<bool()>& send, std::atomic<int>& answers, int expected = 1) -> ::testing::AssertionResult {
        if (!send()) {
            m_client->removeHadlers();
            return ::testing::AssertionFailure() << "not sent";
        }
        const bool ok = WaitFor([&] { return answers >= expected; });
        m_client->removeHadlers();
        if (!ok)
            return ::testing::AssertionFailure() << "no answer";
        return ::testing::AssertionSuccess();
    }

    const std::string m_host = "127.0.0.1";
    const uint16_t m_port = NextPort();
    std::filesystem::path m_dir;
    std::unique_ptr<FakeStreamingServer> m_server;
    std::shared_ptr<ClientNetConfigManager> m_client;
};

}  // namespace

// rpsa_client -c -g V1: the whole config of every host.
TEST_F(AppFlow, GetTheWholeConfig) {
    m_server->config->getSettingsRef().setADCDecimation(32);
    connect();
    std::atomic<int> got{0};
    std::string json;
    m_client->getNewSettingsNofiy.connect([&](std::string& host) {
        json = m_client->getLocalSettingsOfHost(host)->toJson();
        got++;
    });
    ASSERT_TRUE(step([&] { return m_client->requestConfig(m_host); }, got));
    EXPECT_EQ(json, m_server->config->getSettings().toJson());
}

// rpsa_client -c -i key, then -i key=value, then -i key again, in one session.
TEST_F(AppFlow, GetSetGetOneKeyInOneSession) {
    connect();
    auto get = [&](const std::string& key) {
        std::atomic<int> got{0};
        std::string value;
        m_client->getNewSettingsItemNofiy.connect([&](std::string& host) {
            value = m_client->getLocalSettingsOfHost(host)->getValue(key);
            got++;
        });
        EXPECT_TRUE(step([&] { return m_client->requestConfigVariable(m_host, key); }, got));
        return value;
    };
    EXPECT_EQ(get("adc_decimation"), "1");

    std::atomic<int> ok{0}, fail{0};
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    m_client->failSendConfigNofiy.connect([&](std::string&) { fail++; });
    ASSERT_TRUE(step([&] { return m_client->sendConfigVariable(m_host, "adc_decimation", "64"); }, ok));
    EXPECT_EQ(fail, 0);

    EXPECT_EQ(get("adc_decimation"), "64");
    EXPECT_EQ(ok, 1) << "the handlers of the set step were dropped";
    EXPECT_EQ(m_server->config->getSettings().getADCDecimation(), 64u);
}

TEST_F(AppFlow, AnUnknownOrBadKeyIsReportedAsAFailedSet) {
    connect();
    for (auto kv : std::vector<std::pair<std::string, std::string>>{{"no_such_key", "1"}, {"adc_decimation", "0"}, {"resolution", "BIT_7"}}) {
        std::atomic<int> fail{0};
        m_client->failSendConfigNofiy.connect([&](std::string&) { fail++; });
        EXPECT_TRUE(step([&] { return m_client->sendConfigVariable(m_host, kv.first, kv.second); }, fail)) << kv.first;
    }
    EXPECT_EQ(m_server->config->getSettings().toJson(), CStreamSettings().toJson());
}

// rpsa_client -c -s M -f file: the file goes to the server's memory only.
TEST_F(AppFlow, SetTheWholeConfigFromAFileToMemory) {
    connect();
    CStreamSettings wanted;
    wanted.setADCDecimation(256);
    wanted.setADCChannels(1, State::ON);
    wanted.setMemoryBlockSize(0x20000);
    const std::string file = (m_dir / "config.json").string();
    ASSERT_TRUE(wanted.writeToFile(file));

    ASSERT_TRUE(m_client->readFromFile(file));
    std::atomic<int> ok{0};
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    ASSERT_TRUE(step([&] { return m_client->sendConfig(m_host); }, ok));
    EXPECT_EQ(m_server->config->getSettings().toJson(), wanted.toJson());
    EXPECT_EQ(m_server->memChanges, 1) << "streaming-server reallocates its blocks for a whole config";

    CStreamSettings onDisk;
    ASSERT_TRUE(onDisk.readFromFile(serverFile()));
    EXPECT_EQ(onDisk.getADCDecimation(), 1u) << "-s M does not touch the server's file";
}

// rpsa_client -c -s F -f file: set, then the save command from the success handler.
TEST_F(AppFlow, SetTheWholeConfigFromAFileAndSaveIt) {
    connect();
    CStreamSettings wanted;
    wanted.setADCDecimation(128);
    wanted.setGPIODecimation(8);
    const std::string file = (m_dir / "config.json").string();
    ASSERT_TRUE(wanted.writeToFile(file));
    ASSERT_TRUE(m_client->readFromFile(file));

    std::atomic<int> done{0};
    m_client->successSendConfigNofiy.connect([&](std::string& host) { m_client->sendSaveToFile(host); });
    m_client->successSaveConfigNofiy.connect([&](std::string&) { done++; });
    ASSERT_TRUE(step([&] { return m_client->sendConfig(m_host); }, done));

    CStreamSettings onDisk;
    ASSERT_TRUE(onDisk.readFromFile(serverFile()));
    EXPECT_EQ(onDisk.toJson(), wanted.toJson());
}

TEST_F(AppFlow, ASaveThatCannotBeWrittenIsReported) {
    // The settings file of this server sits under a regular file, so it cannot be written.
    const std::string blocker = (m_dir / "not_a_dir").string();
    { std::ofstream(blocker) << "x"; }
    const uint16_t port = NextPort();
    FakeStreamingServer server(blocker + "/streaming_config.json", port);
    std::atomic<bool> connected{false};
    m_client->serverConnectedNofiy.connect([&](std::string&) { connected = true; });
    m_client->connectToServers({m_host}, port);
    ASSERT_TRUE(WaitFor([&] { return connected.load(); }));
    m_client->removeHadlers();

    std::atomic<int> fail{0};
    m_client->failSaveConfigNofiy.connect([&](std::string&) { fail++; });
    EXPECT_TRUE(step([&] { return m_client->sendSaveToFile(m_host); }, fail));
}

// What streaming-server does with block_size: reallocate. Other keys do not.
TEST_F(AppFlow, OnlyTheBlockSizeKeyReallocates) {
    connect();
    std::atomic<int> ok{0};
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    ASSERT_TRUE(step([&] { return m_client->sendConfigVariable(m_host, "adc_decimation", "4"); }, ok));
    EXPECT_EQ(m_server->memChanges, 0);
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    ASSERT_TRUE(step([&] { return m_client->sendConfigVariable(m_host, "block_size", "131072"); }, ok, 2));
    EXPECT_EQ(m_server->memChanges, 1);
    EXPECT_EQ(m_server->newSettings, 2);
}

// rpsa_client -r start / memory block / active channels / ADC FPGA start / stop.
TEST_F(AppFlow, RemoteAdcSessionOverTcp) {
    m_server->config->getSettingsRef().setADCChannels(1, State::ON);
    m_server->config->getSettingsRef().setADCChannels(2, State::ON);
    connect();

    std::atomic<int> started{0};
    m_client->serverStartedTCPNofiy.connect([&](std::string&) { started++; });
    ASSERT_TRUE(step([&] { return m_client->sendADCServerStart(m_host); }, started));

    std::atomic<int> got{0};
    std::string block;
    m_client->getMemBlockSizeNofiy.connect([&](std::string&, std::string size) {
        block = size;
        got++;
    });
    ASSERT_TRUE(step([&] { return m_client->requestMemoryBlockSize(m_host); }, got));
    EXPECT_EQ(block, "65536");

    std::atomic<int> gotChannels{0};
    adc_channels_t channels;
    m_client->getActiveChannelsNofiy.connect([&](std::string&, adc_channels_t ch) {
        channels = ch;
        gotChannels++;
    });
    ASSERT_TRUE(step([&] { return m_client->requestActiveChannels(m_host); }, gotChannels));
    EXPECT_TRUE(channels.isEnabled(ADCChannels::ADC_CH1));
    EXPECT_TRUE(channels.isEnabled(ADCChannels::ADC_CH2));
    EXPECT_FALSE(channels.isEnabled(ADCChannels::ADC_CH3)) << "a 2-channel board reports 2 at most";

    std::atomic<int> adcDone{0};
    m_client->startADCDoneNofiy.connect([&](std::string&) { adcDone++; });
    ASSERT_TRUE(step([&] { return m_client->sendADCFPGAStart(m_host); }, adcDone));

    std::atomic<int> stopped{0};
    m_client->serverStoppedNofiy.connect([&](std::string&) { stopped++; });
    ASSERT_TRUE(step([&] { return m_client->sendADCServerStop(m_host); }, stopped));
    EXPECT_EQ(m_server->adcStarts, 1);
    EXPECT_EQ(m_server->adcStops, 1);
}

TEST_F(AppFlow, RemoteAdcStartInFileModeAndWithoutChannels) {
    connect();
    std::atomic<int> noChannels{0};
    m_client->serverStoppedNoActiveChannelsNofiy.connect([&](std::string&) { noChannels++; });
    ASSERT_TRUE(step([&] { return m_client->sendADCServerStart(m_host); }, noChannels));

    std::atomic<int> ok{0};
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    ASSERT_TRUE(step([&] { return m_client->sendConfigVariable(m_host, "channel_state_1", "ON") && m_client->sendConfigVariable(m_host, "adc_pass_mode", "FILE"); }, ok, 2));

    std::atomic<int> sd{0}, tcp{0};
    m_client->serverStartedSDNofiy.connect([&](std::string&) { sd++; });
    m_client->serverStartedTCPNofiy.connect([&](std::string&) { tcp++; });
    ASSERT_TRUE(step([&] { return m_client->sendADCServerStart(m_host); }, sd));
    EXPECT_EQ(tcp, 0);
}

// rpsa_client -r start for the DAC: the channel set travels with the command.
TEST_F(AppFlow, RemoteDacSession) {
    connect();
    dac_channels_t channels;
    channels.enable(DACChannels::DAC_CH2);
    std::atomic<int> started{0};
    m_client->serverDacStartedNofiy.connect([&](std::string&) { started++; });
    ASSERT_TRUE(step([&] { return m_client->sendDACServerStart(m_host, channels); }, started));
    {
        std::lock_guard lock(m_server->mutex);
        EXPECT_FALSE(m_server->dacChannels.isEnabled(DACChannels::DAC_CH1));
        EXPECT_TRUE(m_server->dacChannels.isEnabled(DACChannels::DAC_CH2));
    }

    std::atomic<int> dacDone{0};
    m_client->startDACDoneNofiy.connect([&](std::string&) { dacDone++; });
    ASSERT_TRUE(step([&] { return m_client->sendDACFPGAStart(m_host); }, dacDone));

    std::atomic<int> stopped{0};
    m_client->serverDacStoppedNofiy.connect([&](std::string&) { stopped++; });
    ASSERT_TRUE(step([&] { return m_client->sendDACServerStop(m_host); }, stopped));

    std::atomic<int> ok{0};
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    ASSERT_TRUE(step([&] { return m_client->sendConfigVariable(m_host, "dac_pass_mode", "DAC_FILE"); }, ok));
    std::atomic<int> sd{0};
    m_client->serverDacStartedSDNofiy.connect([&](std::string&) { sd++; });
    ASSERT_TRUE(step([&] { return m_client->sendDACServerStart(m_host, channels); }, sd));
}

// remote.cpp starts the slaves first, then the masters, by the mode each server announced.
TEST_F(AppFlow, TheServerModeDecidesMasterOrSlave) {
    connect();
    EXPECT_EQ(m_client->getModeByHost(m_host), broadcast_lib::AB_SERVER_MASTER);

    const uint16_t port = NextPort();
    FakeStreamingServer slave((m_dir / "slave.json").string(), port, broadcast_lib::AB_SERVER_SLAVE);
    auto client = std::make_shared<ClientNetConfigManager>("", false);
    std::atomic<bool> connected{false};
    client->serverConnectedNofiy.connect([&](std::string&) { connected = true; });
    client->connectToServers({m_host}, port);
    ASSERT_TRUE(WaitFor([&] { return connected.load(); }));
    EXPECT_EQ(client->getModeByHost(m_host), broadcast_lib::AB_SERVER_SLAVE);
    EXPECT_TRUE(client->isServersConnected());
}

// Every stop reason streaming-server sends reaches its own client signal.
TEST_F(AppFlow, EveryStopReasonReachesItsSignal) {
    connect();
    struct Case {
        const char* name;
        std::function<bool(ServerNetConfigManager&)> send;
        sigslot::signal<std::string&>* signal;
    };
    auto* c = m_client.get();
    const std::vector<Case> cases = {
        {"adc stopped", [](auto& s) { return s.sendServerStopped(); }, &c->serverStoppedNofiy},
        {"adc sd full", [](auto& s) { return s.sendServerStoppedSDFull(); }, &c->serverStoppedSDFullNofiy},
        {"adc sd done", [](auto& s) { return s.sendServerStoppedDone(); }, &c->serverStoppedSDDoneNofiy},
        {"adc mem error", [](auto& s) { return s.sendServerMemoryErrorStopped(); }, &c->serverStoppedMemErrorNofiy},
        {"adc mem modify", [](auto& s) { return s.sendServerMemoryModifyStopped(); }, &c->serverStoppedMemModifyNofiy},
        {"adc no channels", [](auto& s) { return s.sendServerNoChannelsStopped(); }, &c->serverStoppedNoActiveChannelsNofiy},
        {"adc config error", [](auto& s) { return s.sendServerConfigErrorStopped(); }, &c->serverStoppedConfigErrorNofiy},
        {"dac stopped", [](auto& s) { return s.sendDACServerStopped(); }, &c->serverDacStoppedNofiy},
        {"dac mem error", [](auto& s) { return s.sendDACServerMemoryErrorStopped(); }, &c->serverDacStoppedMemErrorNofiy},
        {"dac mem modify", [](auto& s) { return s.sendDACServerMemoryModifyStopped(); }, &c->serverDacStoppedMemModifyNofiy},
        {"dac config error", [](auto& s) { return s.sendDACServerConfigErrorStopped(); }, &c->serverDacStoppedConfigErrorNofiy},
        {"dac sd done", [](auto& s) { return s.sendDACServerStoppedSDDone(); }, &c->serverDacStoppedSDDoneNofiy},
        {"dac sd empty", [](auto& s) { return s.sendDACServerStoppedSDEmpty(); }, &c->serverDacStoppedSDEmptyNofiy},
        {"dac sd broken", [](auto& s) { return s.sendDACServerStoppedSDBroken(); }, &c->serverDacStoppedSDBrokenNofiy},
        {"dac sd missing", [](auto& s) { return s.sendDACServerStoppedSDMissingFile(); }, &c->serverDacStoppedSDMissingNofiy},
        {"config file missed", [](auto& s) { return s.sendConfigFileMissed(); }, &c->configFileMissedNotify},
    };
    std::vector<std::unique_ptr<std::atomic<int>>> hits;
    for (const auto& k : cases) {
        hits.push_back(std::make_unique<std::atomic<int>>(0));
        auto* h = hits.back().get();
        k.signal->connect([h](std::string&) { (*h)++; });
    }
    for (size_t i = 0; i < cases.size(); i++) {
        ASSERT_TRUE(cases[i].send(*m_server->config)) << cases[i].name;
        ASSERT_TRUE(WaitFor([&] { return *hits[i] == 1; })) << cases[i].name;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (size_t i = 0; i < cases.size(); i++)
        EXPECT_EQ(*hits[i], 1) << cases[i].name << ": exactly its own signal";
    m_client->removeHadlers();
}

// rpsa_client runs once per command: a new process connects to the same server each time.
TEST_F(AppFlow, ANewClientPerCommandFindsTheSettingsOfTheLastOne) {
    for (int i = 1; i <= 3; i++) {
        auto client = std::make_shared<ClientNetConfigManager>("", false);
        std::atomic<bool> connected{false};
        std::atomic<int> ok{0};
        client->serverConnectedNofiy.connect([&](std::string&) { connected = true; });
        client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
        client->connectToServers({m_host}, m_port);
        ASSERT_TRUE(WaitFor([&] { return connected.load(); })) << "run " << i;
        ASSERT_TRUE(client->sendConfigVariable(m_host, "adc_decimation", std::to_string(i * 2)));
        ASSERT_TRUE(WaitFor([&] { return ok == 1; })) << "run " << i;
        client->diconnectAll();
    }
    EXPECT_EQ(m_server->config->getSettings().getADCDecimation(), 6u);
    ASSERT_TRUE(WaitFor([&] { return !m_server->config->isConnected(); })) << "the last client left";
}

// The board was rebooted: the server is new, the client connects again.
TEST_F(AppFlow, TheClientReconnectsToARestartedServer) {
    connect();
    std::atomic<int> errors{0};
    m_client->errorNofiy.connect([&](ClientNetConfigManager::Errors, std::string, std::error_code) { errors++; });
    m_server.reset();
    ASSERT_TRUE(WaitFor([&] { return errors > 0; })) << "the client is told the server went away";
    EXPECT_TRUE(m_client->getHosts().empty());
    m_client->removeHadlers();

    m_server = std::make_unique<FakeStreamingServer>(serverFile(), m_port);
    connect();
    std::atomic<int> ok{0};
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    EXPECT_TRUE(step([&] { return m_client->sendConfigVariable(m_host, "adc_decimation", "2"); }, ok));
}

TEST_F(AppFlow, RemoveHadlersSilencesEverySignal) {
    connect();
    std::atomic<int> any{0};
    m_client->serverStoppedNofiy.connect([&](std::string&) { any++; });
    m_client->successSendConfigNofiy.connect([&](std::string&) { any++; });
    m_client->getTriggerStatusNofiy.connect([&](std::string&, TriggerStatus) { any++; });
    m_client->removeHadlers();
    m_server->config->sendServerStopped();
    m_client->sendConfigVariable(m_host, "adc_decimation", "2");
    m_client->requestTriggerStatus(m_host);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_EQ(any, 0);
}
