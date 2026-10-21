/**
 * What the config channel does with input it did not expect, with connections that come and
 * go, and with sends and stops from any thread.
 *
 * A packet this protocol does not produce ends the server on purpose (a peer of another
 * version must not be served), so those cases are death tests: they pin that the process ends
 * at once instead of spinning on a zero size or reading past the packet.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "asio.hpp"
#include "config_net_lib/client_net_config_manager.h"
#include "config_net_lib/config_apply.h"
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

auto NextPort() -> uint16_t {
    static std::atomic<uint16_t> port{29400};
    return port++;
}

auto TempFile(uint16_t port) -> std::string {
    return (std::filesystem::temp_directory_path() / ("config_net_robust_" + std::to_string(port) + ".json")).string();
}

auto Words(std::initializer_list<uint32_t> words) -> std::string {
    std::string s;
    for (uint32_t w : words)
        s.append(reinterpret_cast<const char*>(&w), sizeof(w));
    return s;
}

auto KeyValue(const std::string& key, const std::string& value) -> std::string {
    return Words({0xABCDEF, uint32_t(16 + key.size() + value.size()), uint32_t(key.size()), uint32_t(value.size())}) + key + value;
}

auto Command(uint32_t command, const std::string& tag = "") -> std::string {
    return Words({0xABCDED, uint32_t(16 + tag.size()), command, uint32_t(tag.size())}) + tag;
}

auto Config(const std::string& json) -> std::string {
    return Words({0xABCDEB, uint32_t(12 + json.size()), uint32_t(json.size())}) + json;
}

// A bare TCP peer that writes exactly the bytes it is given.
class RawPeer {
   public:
    auto connect(uint16_t port) -> bool {
        asio::error_code ec;
        m_socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port), ec);
        if (!ec)
            m_socket.non_blocking(true);
        return !ec;
    }
    auto send(const std::string& bytes) -> void {
        m_socket.non_blocking(false);
        asio::write(m_socket, asio::buffer(bytes));
        m_socket.non_blocking(true);
    }
    // Reads until the timeout or until the peer closes; returns true if it closed.
    auto readFor(std::chrono::milliseconds timeout) -> bool {
        const auto end = std::chrono::steady_clock::now() + timeout;
        char buf[4096];
        while (std::chrono::steady_clock::now() < end) {
            asio::error_code ec;
            size_t n = m_socket.read_some(asio::buffer(buf), ec);
            if (ec == asio::error::would_block) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            } else if (ec) {
                return true;
            } else {
                m_received.append(buf, n);
            }
        }
        return false;
    }
    auto close() -> void {
        asio::error_code ec;
        m_socket.close(ec);
    }
    auto received() const -> const std::string& { return m_received; }

   private:
    asio::io_context m_io;
    asio::ip::tcp::socket m_socket{m_io};
    std::string m_received;
};

// Starts a server, sends one packet from a raw peer and waits. For the death tests: if the
// server survives, the process exits normally and the death test fails.
[[noreturn]] auto ServeOnePacket(uint16_t port, const std::string& packet) -> void {
    ServerNetConfigManager server(TempFile(port), broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", port, 2);
    RawPeer peer;
    if (peer.connect(port)) {
        peer.send(packet);
        peer.readFor(std::chrono::milliseconds(2000));
    }
    std::_Exit(0);
}

class Robust : public ::testing::Test {
   protected:
    void SetUp() override {
        m_server = std::make_unique<ServerNetConfigManager>(TempFile(m_port), broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", m_port, 2);
        m_server->clientConnectedNofiy.connect([this]() { m_connected++; });
        m_server->clientDisconnectedNofiy.connect([this]() { m_disconnected++; });
    }
    void TearDown() override {
        m_server.reset();
        std::error_code ec;
        std::filesystem::remove(TempFile(m_port), ec);
    }

    const uint16_t m_port = NextPort();
    std::unique_ptr<ServerNetConfigManager> m_server;
    std::atomic<int> m_connected{0};
    std::atomic<int> m_disconnected{0};
};

}  // namespace

// ---- packets this protocol does not produce ----

TEST(NetConfigPacketDeathTest, AnUnknownHeaderEndsTheServer) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(ServeOnePacket(29380, Words({0x01020304, 16, 0, 0})), "Broken header");
}

TEST(NetConfigPacketDeathTest, AZeroSegmentSizeEndsTheServerInsteadOfSpinning) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(ServeOnePacket(29381, Words({0xABCDEB, 0, 4}) + "abcd"), "Broken segment size");
}

TEST(NetConfigPacketDeathTest, ASegmentShorterThanItsHeaderEndsTheServer) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(ServeOnePacket(29382, Words({0xABCDED, 12, 3})), "Broken segment size");
}

TEST(NetConfigPacketDeathTest, AKeySizePastTheSegmentEndsTheServerWithoutReadingPastIt) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(ServeOnePacket(29383, Words({0xABCDEF, 16, 0x100000, 0x100000})), "Broken segment size");
}

TEST(NetConfigPacketDeathTest, APayloadSizeThatDisagreesWithTheSegmentEndsTheServer) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(ServeOnePacket(29384, Words({0xABCDEB, 20, 4}) + "abcdefgh"), "Broken segment size");
}

// ---- connections ----

// One client at a time: a new one replaces the old, whose unfinished packet must not be
// glued to the new client's data.
TEST_F(Robust, ASecondClientReplacesTheFirstWithoutMixingTheirData) {
    RawPeer first;
    ASSERT_TRUE(first.connect(m_port));
    ASSERT_TRUE(WaitFor([&] { return m_connected == 1; }));
    first.send(KeyValue("adc_decimation", "8").substr(0, 10));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    RawPeer second;
    ASSERT_TRUE(second.connect(m_port));
    ASSERT_TRUE(WaitFor([&] { return m_connected == 2; }));
    second.send(KeyValue("adc_decimation", "4"));
    ASSERT_TRUE(WaitFor([&] { return m_server->getSettings().getADCDecimation() == 4u; }));

    EXPECT_TRUE(first.readFor(std::chrono::milliseconds(1000))) << "the replaced client is closed";
    EXPECT_EQ(m_disconnected, 1);
    EXPECT_TRUE(m_server->isConnected());
}

TEST_F(Robust, TheServerSeesTheClientLeaveAndTakesTheNextOne) {
    {
        RawPeer peer;
        ASSERT_TRUE(peer.connect(m_port));
        ASSERT_TRUE(WaitFor([&] { return m_server->isConnected(); }));
        peer.close();
    }
    ASSERT_TRUE(WaitFor([&] { return m_disconnected == 1; }));
    EXPECT_FALSE(m_server->isConnected());

    RawPeer next;
    ASSERT_TRUE(next.connect(m_port));
    next.send(KeyValue("adc_decimation", "16"));
    ASSERT_TRUE(WaitFor([&] { return m_server->getSettings().getADCDecimation() == 16u; }));
}

// Without the handshake fix a refused client counted as connected and "sent" into the void.
TEST(NetConfigClient, ARefusedConnectionIsNotAConnection) {
    ClientNetConfigManager client("", false);
    std::atomic<int> errors{0};
    client.errorNofiy.connect([&](ClientNetConfigManager::Errors, std::string, std::error_code) { errors++; });
    client.connectToServers({"127.0.0.1"}, NextPort());
    ASSERT_TRUE(WaitFor([&] { return errors > 0; }));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(errors, 1);
    EXPECT_TRUE(client.getHosts().empty());
    EXPECT_FALSE(client.sendADCServerStart("127.0.0.1"));
    EXPECT_TRUE(client.isServersConnected()) << "no connected server is waiting for its handshake";
}

TEST(NetConfigClient, AnUnresolvableHostIsAnErrorNotAnException) {
    ClientNetConfigManager client("", false);
    std::atomic<int> errors{0};
    client.errorNofiy.connect([&](ClientNetConfigManager::Errors e, std::string, std::error_code) {
        if (e == ClientNetConfigManager::Errors::SERVER_INTERNAL)
            errors++;
    });
    EXPECT_NO_THROW(client.connectToServers({"no-such-host.invalid"}, NextPort()));
    ASSERT_TRUE(WaitFor([&] { return errors == 1; }, std::chrono::milliseconds(10000)));
    EXPECT_TRUE(client.getHosts().empty());
}

TEST_F(Robust, TheSettingsFileIsReadBeforeTheFirstClientIsServed) {
    const uint16_t port = NextPort();
    { std::ofstream(TempFile(port)) << R"({"adc_streaming":{"adc_decimation":64},"dac_streaming":{},"memory_manager":{}})"; }
    ServerNetConfigManager server(TempFile(port), broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", port, 2);
    RawPeer peer;
    ASSERT_TRUE(peer.connect(port));
    peer.send(Command(Cmd::CS_REQUEST_SERVER_SETTINGS_VARIABLE, "adc_decimation"));
    peer.readFor(std::chrono::milliseconds(500));
    EXPECT_NE(peer.received().find("adc_decimation64"), std::string::npos);
    std::error_code ec;
    std::filesystem::remove(TempFile(port), ec);
}

TEST_F(Robust, SetBroadcastAddressWithoutABroadcastIsHarmless) {
    EXPECT_NO_THROW(m_server->setBroadcastAddress("127.0.0.1"));
}

// ---- whole configs ----

TEST_F(Robust, AWholeConfigWithARefusedValueChangesNothing) {
    RawPeer peer;
    ASSERT_TRUE(peer.connect(m_port));
    peer.send(Config(R"({"adc_streaming":{"adc_decimation":8},"dac_streaming":{},"memory_manager":{},"gpio_streaming":{"gpio_decimation":0}})"));
    peer.readFor(std::chrono::milliseconds(300));
    EXPECT_NE(peer.received().find(Words({0xABCDED, 16, Cmd::CS_RESPONSE_SETTINGS_GET_FAIL, 0})), std::string::npos);
    EXPECT_EQ(m_server->getSettings().getADCDecimation(), 1u) << "the accepted key of the refused document is not applied either";
    EXPECT_EQ(m_server->getSettings().getGPIODecimation(), 1u);
}

TEST(ApplyConfigJson, EveryGivenValueMustBeTaken) {
    CStreamSettings s;
    EXPECT_TRUE(applyConfigJson(R"({"adc_streaming":{"adc_decimation":8,"adc_trigger_level_low":-100,"samples_limit_sd":18446744073709551615},)"
                                R"("dac_streaming":{"file_sd":"x.wav"},"memory_manager":{}})",
                                s));
    EXPECT_EQ(s.getADCDecimation(), 8u);
    EXPECT_EQ(s.getADCTriggerLevelLow(), -100);
    EXPECT_EQ(s.getADCSamples(), 18446744073709551615ull);

    const std::string before = s.toJson();
    EXPECT_FALSE(applyConfigJson(R"({"adc_streaming":{"adc_decimation":0},"dac_streaming":{},"memory_manager":{}})", s));
    EXPECT_FALSE(applyConfigJson(R"({"adc_streaming":{"adc_decimation":2.5},"dac_streaming":{},"memory_manager":{}})", s)) << "not 2";
    EXPECT_FALSE(applyConfigJson(R"({"adc_streaming":{},"dac_streaming":{"file_type_sd":"BIN"},"memory_manager":{}})", s));
    EXPECT_FALSE(applyConfigJson(R"({"adc_streaming":{"adc_trigger_level_high":40000},"dac_streaming":{},"memory_manager":{}})", s));
    EXPECT_FALSE(applyConfigJson(R"({"adc_streaming":{},"dac_streaming":{},"memory_manager":{},"ext_trigger":{"ext_trig_debounce":70000}})", s));
    EXPECT_EQ(s.toJson(), before);
}

TEST(ApplyConfigJson, UnknownSectionsAndKeysAreIgnored) {
    CStreamSettings s;
    EXPECT_TRUE(applyConfigJson(R"({"adc_streaming":{"adc_decimation":4,"from_the_future":1},"dac_streaming":{},"memory_manager":{},"later":{"x":1}})", s));
    EXPECT_EQ(s.getADCDecimation(), 4u);
}

// ---- sends and stops from any thread ----

// Several threads send large packets at once; each arrives whole and in its thread's order.
TEST(NetConfigTransport, ConcurrentLargeSendsArriveWholeAndInOrder) {
    const uint16_t port = NextPort();
    CNetConfigManager server;
    server.startAsioNet(net_lib::M_SERVER, "127.0.0.1", port);
    CNetConfigManager client;
    std::mutex mutex;
    std::map<int, int> next;
    std::atomic<int> received{0}, broken{0};
    client.receivedCommandNotify.connect([&](uint32_t command, std::string tag) {
        const int thread = command - 1000;
        const size_t colon = tag.find(':');
        const int seq = std::atoi(tag.c_str() + colon + 1);
        const size_t size = 1000 + (seq * 7919 + thread * 104729) % 200000;
        std::lock_guard lock(mutex);
        if (tag.size() != size || tag.find_first_not_of('x', tag.find('|') + 1) != std::string::npos || next[thread] != seq)
            broken++;
        next[thread] = seq + 1;
        received++;
    });
    std::atomic<bool> connected{false};
    server.connectNotify.connect([&](std::string) { connected = true; });
    client.startAsioNet(net_lib::M_CLIENT, "127.0.0.1", port);
    ASSERT_TRUE(WaitFor([&] { return connected.load() && client.isConnected(); }));

    const int threads = 4, perThread = 40;
    std::vector<std::thread> senders;
    for (int t = 0; t < threads; t++) {
        senders.emplace_back([&, t]() {
            for (int seq = 0; seq < perThread; seq++) {
                std::string tag = std::to_string(t) + ":" + std::to_string(seq) + "|";
                const size_t size = 1000 + (seq * 7919 + t * 104729) % 200000;
                tag.append(size - tag.size(), 'x');
                // A mix of the queued and the waiting send.
                server.sendCommand(static_cast<Cmd>(1000 + t), tag, seq % 2 == 0);
            }
        });
    }
    for (auto& s : senders)
        s.join();
    ASSERT_TRUE(WaitFor([&] { return received == threads * perThread; }, std::chrono::milliseconds(20000))) << received;
    EXPECT_EQ(broken, 0);
}

TEST(NetConfigTransport, AWaitingSendToAGonePeerReturnsFalse) {
    const uint16_t port = NextPort();
    CNetConfigManager server;
    server.startAsioNet(net_lib::M_SERVER, "127.0.0.1", port);
    std::atomic<bool> gone{false};
    server.disconnectNotify.connect([&](std::string) { gone = true; });
    {
        RawPeer peer;
        ASSERT_TRUE(peer.connect(port));
        ASSERT_TRUE(WaitFor([&] { return server.isConnected(); }));
        EXPECT_TRUE(server.sendConfig("{}", false));
        peer.close();
    }
    ASSERT_TRUE(WaitFor([&] { return gone.load(); }));
    EXPECT_FALSE(server.sendConfig("{}", false));
}

// Stopping from a handler runs on the asio thread, which cannot join itself.
TEST(NetConfigTransport, StoppingFromInsideAHandlerNeitherHangsNorCrashes) {
    const uint16_t port = NextPort();
    ServerNetConfigManager server(TempFile(port), broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", port, 2);
    auto client = std::make_unique<CNetConfigManager>();
    std::atomic<bool> stopped{false};
    client->receivedCommandNotify.connect([&](uint32_t, std::string) {
        if (!stopped.exchange(true))
            client->stopAsioNet();
    });
    client->startAsioNet(net_lib::M_CLIENT, "127.0.0.1", port);
    ASSERT_TRUE(WaitFor([&] { return stopped.load(); }));
    auto destroyed = std::async(std::launch::async, [&]() { client.reset(); });
    EXPECT_EQ(destroyed.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    std::error_code ec;
    std::filesystem::remove(TempFile(port), ec);
}

TEST(NetConfigTransport, DestroyingTheServerWhileAClientFloodsIsSafe) {
    const uint16_t port = NextPort();
    auto server = std::make_unique<ServerNetConfigManager>(TempFile(port), broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", port, 2);
    ClientNetConfigManager client("", false);
    std::atomic<bool> connected{false};
    client.serverConnectedNofiy.connect([&](std::string&) { connected = true; });
    client.connectToServers({"127.0.0.1"}, port);
    ASSERT_TRUE(WaitFor([&] { return connected.load(); }));
    std::atomic<bool> run{true};
    std::thread flood([&]() {
        while (run)
            client.sendConfigVariable("127.0.0.1", "adc_decimation", "2");
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    server.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    run = false;
    flood.join();
    EXPECT_TRUE(WaitFor([&] { return client.getHosts().empty(); }));
    std::error_code ec;
    std::filesystem::remove(TempFile(port), ec);
}

TEST(NetConfigTransport, ReconnectingTheClientListFromAHandlerIsSafe) {
    const uint16_t port = NextPort();
    ServerNetConfigManager server(TempFile(port), broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", port, 2);
    ClientNetConfigManager client("", false);
    std::atomic<int> connects{0};
    client.serverConnectedNofiy.connect([&](std::string&) {
        // Replacing the list destroys the connection this handler runs on.
        if (connects++ == 0)
            client.connectToServers({"127.0.0.1"}, port);
    });
    client.connectToServers({"127.0.0.1"}, port);
    ASSERT_TRUE(WaitFor([&] { return connects == 2; }));
    std::error_code ec;
    std::filesystem::remove(TempFile(port), ec);
}

// ---- broadcast ----

TEST(Broadcast, AMessageIsFourFieldsWithAnOptionalTrailingNewline) {
    ClientNetConfigManager::BroadCastClients c;
    ASSERT_TRUE(ClientNetConfigManager::parseBroadcast("00:26:32:f0:00:01\n200.0.0.33\nM\n7\n", c));
    EXPECT_EQ(c.mac, "00:26:32:f0:00:01");
    EXPECT_EQ(c.host, "200.0.0.33");
    EXPECT_EQ(c.mode, broadcast_lib::AB_SERVER_MASTER);
    EXPECT_EQ(c.model, 7);

    ASSERT_TRUE(ClientNetConfigManager::parseBroadcast("mac\nhost\nS\n12", c)) << "the last field ends at the message end";
    EXPECT_EQ(c.mode, broadcast_lib::AB_SERVER_SLAVE);
    EXPECT_EQ(c.model, 12);

    ASSERT_TRUE(ClientNetConfigManager::parseBroadcast("mac\nhost\nC\n1\n", c));
    EXPECT_EQ(c.mode, broadcast_lib::AB_NONE);

    for (const char* bad : {"", "\n", "mac\nhost\nM", "mac\nhost\nM\n7\nextra", "mac\nhost\nM\n7\n\n\n"}) {
        EXPECT_FALSE(ClientNetConfigManager::parseBroadcast(bad, c)) << bad;
    }
}
