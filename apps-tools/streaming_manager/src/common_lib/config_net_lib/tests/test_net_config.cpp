/**
 * ServerNetConfigManager and ClientNetConfigManager talking over localhost.
 *
 * The command numbers are the protocol shared with older builds, so they are pinned one by
 * one. The rest runs a real server and client on 127.0.0.1: settings by key and as a whole,
 * the commands added for triggers and GPIO streaming, and the answers a server gives when
 * nothing handles a request.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "config_net_lib/client_net_config_manager.h"
#include "config_net_lib/server_net_config_manager.h"

using Cmd = CNetConfigManager::ECommands;

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

// A port per fixture instance, so a socket still closing cannot break the next test.
auto NextPort() -> uint16_t {
    static std::atomic<uint16_t> port{28900};
    return port++;
}

// Commands received by a client, in order, with their tags.
struct Received {
    std::mutex mutex;
    std::vector<std::pair<uint32_t, std::string>> commands;

    auto add(uint32_t command, const std::string& tag) -> void {
        std::lock_guard lock(mutex);
        commands.emplace_back(command, tag);
    }
    auto has(uint32_t command, std::string* tag = nullptr) -> bool {
        std::lock_guard lock(mutex);
        for (const auto& c : commands) {
            if (c.first == command) {
                if (tag)
                    *tag = c.second;
                return true;
            }
        }
        return false;
    }
};

class NetConfig : public ::testing::Test {
   protected:
    void SetUp() override {
        m_dir = std::filesystem::temp_directory_path() / ("config_net_lib_test_" + std::to_string(m_port));
        std::filesystem::create_directories(m_dir);
        m_server = std::make_unique<ServerNetConfigManager>((m_dir / "server.json").string(), broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", m_port, 2);
    }

    void TearDown() override {
        if (m_client)
            m_client->diconnectAll();
        m_client.reset();
        if (m_raw)
            m_raw->stopAsioNet();
        m_raw.reset();
        m_server.reset();
        std::error_code ec;
        std::filesystem::remove_all(m_dir, ec);
    }

    // The ClientNetConfigManager used by rpsa_client and the streaming API.
    auto connectClient() -> void {
        m_client = std::make_unique<ClientNetConfigManager>((m_dir / "client.json").string(), false);
        std::atomic<bool> connected{false};
        m_client->serverConnectedNofiy.connect([&](std::string&) { connected = true; });
        m_client->connectToServers({"127.0.0.1"}, m_port);
        ASSERT_TRUE(WaitFor([&] { return connected.load(); })) << "client not connected";
        m_client->serverConnectedNofiy.disconnect_all();
    }

    // A bare protocol endpoint, for what ClientNetConfigManager cannot send: an older client's
    // config, a malformed tag.
    auto connectRaw() -> void {
        m_raw = std::make_unique<CNetConfigManager>();
        m_raw->receivedCommandNotify.connect([this](uint32_t command, std::string tag) { m_rawReceived.add(command, tag); });
        m_raw->startAsioNet(net_lib::M_CLIENT, "127.0.0.1", m_port);
        ASSERT_TRUE(WaitFor([&] { return m_rawReceived.has(Cmd::CS_RESPONSE_MASTER_CONNECTED); })) << "raw client not connected";
    }

    const std::string m_host = "127.0.0.1";
    const uint16_t m_port = NextPort();
    std::filesystem::path m_dir;
    std::unique_ptr<ServerNetConfigManager> m_server;
    std::unique_ptr<ClientNetConfigManager> m_client;
    std::unique_ptr<CNetConfigManager> m_raw;
    Received m_rawReceived;
};

}  // namespace

TEST(NetConfigProtocol, TheCommandNumbersAreFixed) {
    const std::vector<std::pair<Cmd, uint32_t>> numbers = {
        {Cmd::CS_RESPONSE_SETTINGS_GET_SUCCESS, 0},
        {Cmd::CS_RESPONSE_SETTINGS_GET_FAIL, 1},
        {Cmd::CS_REQUEST_SERVER_SETTINGS, 2},
        {Cmd::CS_REQUEST_SERVER_SETTINGS_VARIABLE, 3},
        {Cmd::CS_RESPONSE_CONFIG_FILE_MISSED, 4},
        {Cmd::CS_REQUEST_SERVER_MEM_BLOCK_SIZE, 5},
        {Cmd::CS_RESPONSE_MEM_BLOCK_SIZE, 6},
        {Cmd::CS_REQUEST_SERVER_ACTIVE_CHANNELS, 7},
        {Cmd::CS_RESPONSE_ACTIVE_CHANNELS, 8},
        {Cmd::CS_REQUEST_SAVE_SETTING_TO_FILE, 9},
        {Cmd::CS_RESPONSE_SAVE_SETTING_TO_FILE_SUCCES, 10},
        {Cmd::CS_RESPONSE_SAVE_SETTING_TO_FILE_FAIL, 11},
        {Cmd::CS_REQUEST_LOAD_SETTING_FROM_FILE, 12},
        {Cmd::CS_RESPONSE_LOAD_SETTING_FROM_FILE_SUCCES, 13},
        {Cmd::CS_RESPONSE_LOAD_SETTING_FROM_FILE_FAIL, 14},
        {Cmd::CS_RESPONSE_MASTER_CONNECTED, 15},
        {Cmd::CS_RESPONSE_SLAVE_CONNECTED, 16},
        {Cmd::CS_REQUEST_ADC_SERVER_START, 17},
        {Cmd::CS_REQUEST_ADC_SERVER_STOP, 18},
        {Cmd::CS_RESPONSE_ADC_SERVER_STOPPED, 19},
        {Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_NO_CHANNELS, 20},
        {Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_ERROR, 21},
        {Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY, 22},
        {Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_SD_FULL, 23},
        {Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_SD_DONE, 24},
        {Cmd::CS_RESPONSE_ADC_SERVER_STARTED_TCP, 25},
        {Cmd::CS_RESPONSE_ADC_SERVER_STARTED_SD, 26},
        {Cmd::CS_REQUEST_ADC_FPGA_START, 27},
        {Cmd::CS_RESPONSE_ADC_FPGA_START_ADC_DONE, 28},
        {Cmd::CS_REQUEST_DAC_FPGA_START, 29},
        {Cmd::CS_RESPONSE_DAC_FPGA_START_DONE, 30},
        {Cmd::CS_REQUEST_DAC_SERVER_START, 31},
        {Cmd::CS_REQUEST_DAC_SERVER_STOP, 32},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED, 33},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_MEM_ERROR, 34},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_MEM_MODIFY, 35},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_CONFIG_ERROR, 36},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_SD_DONE, 37},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_SD_EMPTY, 38},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_SD_BROKEN, 39},
        {Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_SD_MISSING, 40},
        {Cmd::CS_RESPONSE_DAC_SERVER_STARTED, 41},
        {Cmd::CS_RESPONSE_DAC_SERVER_STARTED_SD, 42},
        {Cmd::CS_REQUEST_GET_SERVER_MODE, 43},
        {Cmd::CS_RESPONSE_SERVER_MODE_TCP, 44},
        {Cmd::CS_RESPONSE_SERVER_MODE_SD, 45},
        {Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_CONFIG_ERROR, 46},
        {Cmd::CS_REQUEST_SOFTWARE_TRIGGER, 47},
        {Cmd::CS_RESPONSE_SOFTWARE_TRIGGER_DONE, 48},
        {Cmd::CS_RESPONSE_SOFTWARE_TRIGGER_FAIL, 49},
        {Cmd::CS_REQUEST_TRIGGER_STATUS, 50},
        {Cmd::CS_RESPONSE_TRIGGER_STATUS, 51},
        {Cmd::CS_REQUEST_GPIO_SERVER_START, 52},
        {Cmd::CS_REQUEST_GPIO_SERVER_STOP, 53},
        {Cmd::CS_RESPONSE_GPIO_SERVER_STARTED, 54},
        {Cmd::CS_RESPONSE_GPIO_SERVER_STOPPED, 55},
        {Cmd::CS_RESPONSE_GPIO_SERVER_STOPPED_MEM_ERROR, 56},
        {Cmd::CS_RESPONSE_GPIO_SERVER_STOPPED_CONFIG_ERROR, 57},
    };
    for (size_t i = 0; i < numbers.size(); i++) {
        EXPECT_EQ(static_cast<uint32_t>(numbers[i].first), numbers[i].second);
        EXPECT_EQ(numbers[i].second, i) << "the numbers have no gaps";
    }
    EXPECT_EQ(static_cast<uint32_t>(CNetConfigManager::TT_ADC), 0u);
    EXPECT_EQ(static_cast<uint32_t>(CNetConfigManager::TT_DAC), 1u);
    EXPECT_EQ(static_cast<uint32_t>(CNetConfigManager::TT_GPIO), 2u);
    EXPECT_EQ(static_cast<int>(ServerNetConfigManager::NO_CHANNELS), 5);
    EXPECT_EQ(static_cast<int>(ServerNetConfigManager::CONFIG_ERROR), 6);
    EXPECT_EQ(static_cast<int>(ClientNetConfigManager::Errors::CONNECT_TIMEOUT), 5);
}

TEST(NetConfigProtocol, TheTriggerTargetTagIsADecimalNumberOnly) {
    CNetConfigManager::ETriggerTarget target = CNetConfigManager::TT_ADC;
    EXPECT_TRUE(CNetConfigManager::parseTriggerTarget("2", target));
    EXPECT_EQ(target, CNetConfigManager::TT_GPIO);
    EXPECT_TRUE(CNetConfigManager::parseTriggerTarget("1", target));
    EXPECT_EQ(target, CNetConfigManager::TT_DAC);
    for (const char* bad : {"", "3", "-1", "01", " 1", "ADC", "0x1"}) {
        EXPECT_FALSE(CNetConfigManager::parseTriggerTarget(bad, target)) << bad;
    }
    EXPECT_EQ(target, CNetConfigManager::TT_DAC) << "a rejected tag leaves the target alone";
}

TEST_F(NetConfig, ANewKeyIsSetAndReadBackByName) {
    connectClient();
    std::atomic<int> ok{0};
    std::atomic<bool> item{false};
    m_client->successSendConfigNofiy.connect([&](std::string&) { ok++; });
    m_client->getNewSettingsItemNofiy.connect([&](std::string&) { item = true; });

    ASSERT_TRUE(m_client->sendConfigVariable(m_host, "adc_trigger_level_low", "-100"));
    ASSERT_TRUE(WaitFor([&] { return ok == 1; }));
    EXPECT_EQ(m_server->getSettings().getADCTriggerLevelLow(), -100);

    ASSERT_TRUE(m_client->sendConfigVariable(m_host, "gpio_event_source", "OSC2"));
    ASSERT_TRUE(WaitFor([&] { return ok == 2; }));

    ASSERT_TRUE(m_client->requestConfigVariable(m_host, "gpio_event_source"));
    ASSERT_TRUE(WaitFor([&] { return item.load(); }));
    EXPECT_EQ(m_client->getLocalSettingsOfHost(m_host)->getGPIOEventSource(), CStreamSettings::EventSource::OSC2);
}

TEST_F(NetConfig, ABadValueIsRefusedAndNothingChanges) {
    connectClient();
    std::atomic<bool> fail{false};
    m_client->failSendConfigNofiy.connect([&](std::string&) { fail = true; });
    ASSERT_TRUE(m_client->sendConfigVariable(m_host, "gpio_decimation", "0"));
    ASSERT_TRUE(WaitFor([&] { return fail.load(); }));
    EXPECT_EQ(m_server->getSettings().getGPIODecimation(), 1u);
}

TEST_F(NetConfig, TheWholeConfigTravelsBothWays) {
    connectClient();
    std::atomic<bool> sent{false}, received{false};
    m_client->successSendConfigNofiy.connect([&](std::string&) { sent = true; });
    m_client->getNewSettingsNofiy.connect([&](std::string&) { received = true; });

    m_client->setExtTrigDebounce(321);
    m_client->setDACTriggerMode(2, CStreamSettings::State::ON);
    ASSERT_TRUE(m_client->sendConfig(m_host));
    ASSERT_TRUE(WaitFor([&] { return sent.load(); }));
    EXPECT_EQ(m_server->getSettings().toJson(), m_client->toJson());

    m_server->getSettingsRef().setGPIOPatternMask(0x0F0F);
    ASSERT_TRUE(m_client->requestConfig(m_host));
    ASSERT_TRUE(WaitFor([&] { return received.load(); }));
    EXPECT_EQ(m_client->getLocalSettingsOfHost(m_host)->toJson(), m_server->getSettings().toJson());
}

// A client built before ext_trigger and gpio_streaming existed sends only the three old
// sections; the server keeps what it had for the rest instead of resetting it.
TEST_F(NetConfig, AnOldClientConfigIsAppliedOverTheCurrentSettings) {
    m_server->getSettingsRef().setGPIODecimation(16);
    m_server->getSettingsRef().setADCTriggerSources(0x20);
    connectRaw();
    ASSERT_TRUE(m_raw->sendConfig(R"({"adc_streaming":{"adc_decimation":8},"dac_streaming":{},"memory_manager":{}})"));
    ASSERT_TRUE(WaitFor([&] { return m_rawReceived.has(Cmd::CS_RESPONSE_SETTINGS_GET_SUCCESS); }));
    EXPECT_EQ(m_server->getSettings().getADCDecimation(), 8u);
    EXPECT_EQ(m_server->getSettings().getGPIODecimation(), 16u);
    EXPECT_EQ(m_server->getSettings().getADCTriggerSources(), 0x20u);
}

TEST_F(NetConfig, ARejectedConfigLeavesTheSettingsAlone) {
    m_server->getSettingsRef().setADCDecimation(4);
    connectRaw();
    ASSERT_TRUE(m_raw->sendConfig(R"({"adc_streaming":{"adc_decimation":8,"channel_state_1":"MAYBE"},"dac_streaming":{},"memory_manager":{}})"));
    ASSERT_TRUE(WaitFor([&] { return m_rawReceived.has(Cmd::CS_RESPONSE_SETTINGS_GET_FAIL); }));
    EXPECT_EQ(m_server->getSettings().getADCDecimation(), 4u) << "a partly applied document must not leak";
}

TEST_F(NetConfig, ASoftwareTriggerWithoutAHandlerFails) {
    connectClient();
    std::atomic<int> failed{-1};
    m_client->softwareTriggerFailNofiy.connect([&](std::string&, CNetConfigManager::ETriggerTarget t) { failed = t; });
    ASSERT_TRUE(m_client->sendSoftwareTrigger(m_host, CNetConfigManager::TT_DAC));
    ASSERT_TRUE(WaitFor([&] { return failed != -1; }));
    EXPECT_EQ(failed, CNetConfigManager::TT_DAC);
}

TEST_F(NetConfig, ASoftwareTriggerReachesTheHandlerWithItsTarget) {
    std::atomic<int> handled{-1};
    m_server->softwareTriggerNofiy.connect([&](CNetConfigManager::ETriggerTarget t) {
        handled = t;
        m_server->sendSoftwareTriggerDone(t);
    });
    connectClient();
    std::atomic<int> done{-1};
    m_client->softwareTriggerDoneNofiy.connect([&](std::string&, CNetConfigManager::ETriggerTarget t) { done = t; });
    ASSERT_TRUE(m_client->sendSoftwareTrigger(m_host, CNetConfigManager::TT_GPIO));
    ASSERT_TRUE(WaitFor([&] { return done != -1; }));
    EXPECT_EQ(handled, CNetConfigManager::TT_GPIO);
    EXPECT_EQ(done, CNetConfigManager::TT_GPIO);
}

TEST_F(NetConfig, AnUnknownTriggerTargetIsRefusedWithItsTag) {
    std::atomic<bool> handled{false};
    m_server->softwareTriggerNofiy.connect([&](CNetConfigManager::ETriggerTarget) { handled = true; });
    connectRaw();
    ASSERT_TRUE(m_raw->sendCommand(Cmd::CS_REQUEST_SOFTWARE_TRIGGER, "7"));
    std::string tag;
    ASSERT_TRUE(WaitFor([&] { return m_rawReceived.has(Cmd::CS_RESPONSE_SOFTWARE_TRIGGER_FAIL, &tag); }));
    EXPECT_EQ(tag, "7");
    EXPECT_FALSE(handled.load());
}

TEST_F(NetConfig, TheTriggerStatusTravelsAsJson) {
    TriggerStatus status;
    status.adcArmed = true;
    status.adcTriggerCount = 42;
    status.dacArmed = 3;
    status.gpioInTriggerCount = 5;
    m_server->triggerStatusRequestNofiy.connect([&]() { m_server->sendTriggerStatus(status); });
    connectClient();
    std::mutex mutex;
    bool got = false;
    TriggerStatus received;
    m_client->getTriggerStatusNofiy.connect([&](std::string&, TriggerStatus s) {
        std::lock_guard lock(mutex);
        received = s;
        got = true;
    });
    ASSERT_TRUE(m_client->requestTriggerStatus(m_host));
    ASSERT_TRUE(WaitFor([&] {
        std::lock_guard lock(mutex);
        return got;
    }));
    std::lock_guard lock(mutex);
    EXPECT_EQ(received, status);
}

TEST_F(NetConfig, NoTriggerStatusWithoutAHandler) {
    connectClient();
    std::atomic<bool> failed{false}, got{false};
    m_client->failTriggerStatusNofiy.connect([&](std::string&) { failed = true; });
    m_client->getTriggerStatusNofiy.connect([&](std::string&, TriggerStatus) { got = true; });
    ASSERT_TRUE(m_client->requestTriggerStatus(m_host));
    ASSERT_TRUE(WaitFor([&] { return failed.load(); }));
    EXPECT_FALSE(got.load());
}

TEST_F(NetConfig, GpioStreamingWithoutAHandlerIsAConfigError) {
    connectClient();
    std::atomic<bool> error{false}, stopped{false};
    m_client->serverGpioStoppedConfigErrorNofiy.connect([&](std::string&) { error = true; });
    m_client->serverGpioStoppedNofiy.connect([&](std::string&) { stopped = true; });
    ASSERT_TRUE(m_client->sendGPIOServerStart(m_host));
    ASSERT_TRUE(WaitFor([&] { return error.load(); }));
    ASSERT_TRUE(m_client->sendGPIOServerStop(m_host));
    ASSERT_TRUE(WaitFor([&] { return stopped.load(); })) << "a stop is always confirmed";
}

TEST_F(NetConfig, GpioStreamingStartAndStopReachTheHandlers) {
    std::atomic<int> starts{0}, stops{0};
    m_server->startGpioStreamingNofiy.connect([&]() {
        starts++;
        m_server->sendGPIOServerStarted();
    });
    m_server->stopGpioStreamingNofiy.connect([&]() {
        stops++;
        m_server->sendGPIOServerMemoryErrorStopped();
    });
    connectClient();
    std::atomic<bool> started{false}, memError{false};
    m_client->serverGpioStartedNofiy.connect([&](std::string&) { started = true; });
    m_client->serverGpioStoppedMemErrorNofiy.connect([&](std::string&) { memError = true; });
    ASSERT_TRUE(m_client->sendGPIOServerStart(m_host));
    ASSERT_TRUE(WaitFor([&] { return started.load(); }));
    ASSERT_TRUE(m_client->sendGPIOServerStop(m_host));
    ASSERT_TRUE(WaitFor([&] { return memError.load(); }));
    EXPECT_EQ(starts, 1);
    EXPECT_EQ(stops, 1);
}

TEST_F(NetConfig, AnAdcConfigErrorReachesTheClient) {
    connectClient();
    std::atomic<bool> error{false}, stopped{false};
    m_client->serverStoppedConfigErrorNofiy.connect([&](std::string&) { error = true; });
    m_client->serverStoppedNofiy.connect([&](std::string&) { stopped = true; });
    ASSERT_TRUE(m_server->sendServerConfigErrorStopped());
    ASSERT_TRUE(WaitFor([&] { return error.load(); }));
    EXPECT_FALSE(stopped.load()) << "not reported as a normal stop";
}

// The client used to swap the two answers.
TEST_F(NetConfig, TheServerModeIsReportedAsItIs) {
    connectClient();
    std::atomic<int> tcp{0}, sd{0};
    m_client->serverModeTCPNofiy.connect([&](std::string&) { tcp++; });
    m_client->serverModeSDNofiy.connect([&](std::string&) { sd++; });

    m_server->getSettingsRef().setADCPassMode(CStreamSettings::PassMode::NET);
    ASSERT_TRUE(m_client->sendGetServerMode(m_host));
    ASSERT_TRUE(WaitFor([&] { return tcp + sd == 1; }));
    EXPECT_EQ(tcp, 1);

    m_server->getSettingsRef().setADCPassMode(CStreamSettings::PassMode::FILE);
    ASSERT_TRUE(m_client->sendGetServerMode(m_host));
    ASSERT_TRUE(WaitFor([&] { return tcp + sd == 2; }));
    EXPECT_EQ(sd, 1);
}

// sendDACStarted used to send the ADC answer.
TEST_F(NetConfig, TheDacStartIsConfirmedAsADacStart) {
    connectClient();
    std::atomic<bool> dac{false}, adc{false};
    m_client->startDACDoneNofiy.connect([&](std::string&) { dac = true; });
    m_client->startADCDoneNofiy.connect([&](std::string&) { adc = true; });
    ASSERT_TRUE(m_server->sendDACStarted());
    ASSERT_TRUE(WaitFor([&] { return dac.load(); }));
    EXPECT_FALSE(adc.load());
}
