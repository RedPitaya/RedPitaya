/**
 * broadcast_lib over the loopback broadcast address 127.255.255.255.
 *
 * The interface list is replaced by a provider, so the announced addresses are known and
 * nothing leaves the machine.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "asio.hpp"
#include "broadcast_lib/asio_broadcast_socket.h"

using namespace broadcast_lib;
using namespace std::chrono_literals;

namespace {

const char* kLoopbackBroadcast = "127.255.255.255";

auto WaitFor(const std::function<bool()>& done, std::chrono::milliseconds timeout = 3000ms) -> bool {
    const auto end = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < end) {
        if (done())
            return true;
        std::this_thread::sleep_for(5ms);
    }
    return done();
}

auto NextPort() -> uint16_t {
    static std::atomic<uint16_t> port{27900};
    return port++;
}

auto Split(const std::string& s) -> std::vector<std::string> {
    std::vector<std::string> fields;
    size_t begin = 0;
    while (begin < s.size()) {
        size_t end = s.find('\n', begin);
        if (end == std::string::npos)
            end = s.size();
        fields.push_back(s.substr(begin, end - begin));
        begin = end + 1;
    }
    return fields;
}

// A client that keeps what it hears.
struct Listener {
    CAsioBroadcastSocket::Ptr socket;
    std::mutex mutex;
    std::vector<std::string> messages;
    std::atomic<int> errors{0};

    explicit Listener(uint16_t port) {
        socket = CAsioBroadcastSocket::create(0, "", "", port);
        socket->receivedNotify.connect([this](std::error_code, uint8_t* buf, size_t n) {
            std::lock_guard lock(mutex);
            messages.emplace_back(reinterpret_cast<char*>(buf), n);
        });
        socket->errorNotify.connect([this](std::error_code) { errors++; });
        socket->initClient();
    }
    auto count() -> size_t {
        std::lock_guard lock(mutex);
        return messages.size();
    }
    auto hosts() -> std::set<std::string> {
        std::lock_guard lock(mutex);
        std::set<std::string> h;
        for (const auto& m : messages) {
            auto f = Split(m);
            if (f.size() == 4)
                h.insert(f[1]);
        }
        return h;
    }
    auto last() -> std::string {
        std::lock_guard lock(mutex);
        return messages.empty() ? "" : messages.back();
    }
    auto clear() -> void {
        std::lock_guard lock(mutex);
        messages.clear();
    }
};

// A server announcing the targets it is given.
struct Announcer {
    CAsioBroadcastSocket::Ptr socket;
    std::mutex mutex;
    std::vector<BroadcastTarget> targets;
    std::atomic<int> sent{0}, errors{0};

    Announcer(uint16_t port, std::vector<BroadcastTarget> _targets, const std::string& host = "192.0.2.1") : targets(std::move(_targets)) {
        socket = CAsioBroadcastSocket::create(7, "00:26:32:f0:00:01", host, port);
        socket->sendNotify.connect([this](std::error_code e, size_t) {
            if (!e)
                sent++;
        });
        socket->errorNotify.connect([this](std::error_code) { errors++; });
        socket->setTargetProvider([this]() {
            std::lock_guard lock(mutex);
            return targets;
        });
    }
    auto setTargets(std::vector<BroadcastTarget> t) -> void {
        std::lock_guard lock(mutex);
        targets = std::move(t);
    }
};

}  // namespace

TEST(Broadcast, TheAnnouncementIsMacHostModeModel) {
    const uint16_t port = NextPort();
    Listener listener(port);
    Announcer server(port, {{"10.0.0.33", kLoopbackBroadcast}});
    server.socket->initServer(AB_SERVER_SLAVE, 50);
    ASSERT_TRUE(WaitFor([&] { return listener.count() > 0; }));
    EXPECT_EQ(listener.last(), "00:26:32:f0:00:01\n10.0.0.33\nS\n7\n");
}

// The options used to be set after bind, so the second client threw "address in use".
TEST(Broadcast, TwoClientsOnOneMachineBothHearTheServer) {
    const uint16_t port = NextPort();
    std::unique_ptr<Listener> first, second;
    ASSERT_NO_THROW(first = std::make_unique<Listener>(port));
    ASSERT_NO_THROW(second = std::make_unique<Listener>(port));
    Announcer server(port, {{"10.0.0.33", kLoopbackBroadcast}});
    server.socket->initServer(AB_SERVER_MASTER, 50);
    EXPECT_TRUE(WaitFor([&] { return first->count() > 0 && second->count() > 0; }));
    EXPECT_EQ(first->errors, 0);
    EXPECT_EQ(second->errors, 0);
}

TEST(Broadcast, ABusyPortIsAnErrorNotAnException) {
    const uint16_t port = NextPort();
    asio::io_context io;
    asio::ip::udp::socket blocker(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port));  // no reuse_address
    std::unique_ptr<Listener> listener;
    ASSERT_NO_THROW(listener = std::make_unique<Listener>(port));
    EXPECT_EQ(listener->errors, 1);
}

TEST(Broadcast, EveryInterfaceIsAnnouncedWithItsOwnAddress) {
    const uint16_t port = NextPort();
    Listener listener(port);
    Announcer server(port, {{"10.0.0.33", kLoopbackBroadcast}, {"10.1.0.33", kLoopbackBroadcast}});
    server.socket->initServer(AB_SERVER_MASTER, 50);
    EXPECT_TRUE(WaitFor([&] { return listener.hosts() == std::set<std::string>{"10.0.0.33", "10.1.0.33"}; }));
}

// The address is read every period: a board that got its address after the start, or a new
// one, is announced with it.
TEST(Broadcast, TheAnnouncedAddressFollowsTheInterface) {
    const uint16_t port = NextPort();
    Listener listener(port);
    Announcer server(port, {});
    server.setTargets({{"10.0.0.33", kLoopbackBroadcast}});
    server.socket->initServer(AB_SERVER_MASTER, 50);
    ASSERT_TRUE(WaitFor([&] { return listener.hosts().count("10.0.0.33") == 1; }));
    server.setTargets({{"10.0.0.44", kLoopbackBroadcast}});
    std::this_thread::sleep_for(100ms);
    listener.clear();
    ASSERT_TRUE(WaitFor([&] { return listener.count() >= 3; }));
    EXPECT_EQ(listener.hosts(), std::set<std::string>{"10.0.0.44"});
}

// An empty address stands for the configured host, which setIPAddress may change at any time
// from any thread; every datagram still carries a whole, valid text.
TEST(Broadcast, TheConfiguredHostCanChangeWhileAnnouncing) {
    const uint16_t port = NextPort();
    Listener listener(port);
    Announcer server(port, {{"", kLoopbackBroadcast}}, "10.9.9.0");
    server.socket->initServer(AB_SERVER_MASTER, 2);
    std::set<std::string> used{"10.9.9.0"};
    for (int i = 1; i <= 200; i++) {
        const std::string host = "10.9.9." + std::to_string(i);
        used.insert(host);
        server.socket->setIPAddress(host);
        if (i % 20 == 0)
            std::this_thread::sleep_for(2ms);
    }
    ASSERT_TRUE(WaitFor([&] { return listener.hosts().count("10.9.9.200") == 1; }));
    std::lock_guard lock(listener.mutex);
    for (const auto& m : listener.messages) {
        auto f = Split(m);
        ASSERT_EQ(f.size(), 4u) << m;
        EXPECT_EQ(used.count(f[1]), 1u) << m;
    }
}

TEST(Broadcast, CloseStopsTheServerForGood) {
    const uint16_t port = NextPort();
    Announcer server(port, {{"10.0.0.33", kLoopbackBroadcast}});
    server.socket->initServer(AB_SERVER_MASTER, 20);
    ASSERT_TRUE(WaitFor([&] { return server.sent > 2; }));
    server.socket->closeSocket();
    const int sent = server.sent;
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(server.sent, sent) << "nothing is sent after closeSocket, the socket does not come back";
    EXPECT_EQ(server.errors, 0) << "closing is not an error";

    server.socket->initServer(AB_SERVER_MASTER, 20);
    EXPECT_TRUE(WaitFor([&] { return server.sent > sent; })) << "it can be started again";
}

TEST(Broadcast, CloseStopsTheClientWithoutAnError) {
    const uint16_t port = NextPort();
    Listener listener(port);
    Announcer server(port, {{"10.0.0.33", kLoopbackBroadcast}});
    server.socket->initServer(AB_SERVER_MASTER, 20);
    ASSERT_TRUE(WaitFor([&] { return listener.count() > 0; }));
    listener.socket->closeSocket();
    const size_t received = listener.count();
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(listener.count(), received);
    EXPECT_EQ(listener.errors, 0) << "operation_aborted of a close is not reported";

    // The socket is really closed: the port takes a socket without reuse_address again.
    server.socket->closeSocket();
    asio::io_context io;
    asio::ip::udp::socket probe(io);
    std::error_code ec;
    probe.open(asio::ip::udp::v4(), ec);
    probe.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port), ec);
    EXPECT_FALSE(ec) << ec.message();
}

TEST(Broadcast, StartingTwiceDoesNotDoubleTheRate) {
    const uint16_t port = NextPort();
    Announcer server(port, {{"10.0.0.33", kLoopbackBroadcast}});
    server.socket->initServer(AB_SERVER_MASTER, 100);
    server.socket->initServer(AB_SERVER_MASTER, 100);
    std::this_thread::sleep_for(550ms);
    EXPECT_LE(server.sent, 7) << "about 6 rounds of one datagram";
}

// The pause between announcements used to be a sleep on the asio thread.
TEST(Broadcast, DestructionDoesNotWaitForThePeriod) {
    const uint16_t port = NextPort();
    auto server = std::make_unique<Announcer>(port, std::vector<BroadcastTarget>{{"10.0.0.33", kLoopbackBroadcast}});
    server->socket->initServer(AB_SERVER_MASTER, 5000);
    ASSERT_TRUE(WaitFor([&] { return server->sent > 0; }));
    const auto start = std::chrono::steady_clock::now();
    server.reset();
    EXPECT_LT(std::chrono::steady_clock::now() - start, 500ms);
}

// Without a network every round fails the same way: reported once, again only after a send
// went through.
TEST(Broadcast, ALastingErrorIsReportedOnce) {
    const uint16_t port = NextPort();
    Announcer server(port, {{"10.0.0.33", "not-an-address"}});
    server.socket->initServer(AB_SERVER_MASTER, 10);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(server.errors, 1);

    server.setTargets({{"10.0.0.33", kLoopbackBroadcast}});
    ASSERT_TRUE(WaitFor([&] { return server.sent > 0; }));
    server.setTargets({{"10.0.0.33", "not-an-address"}});
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(server.errors, 1) << "one destination, one lasting error";
}

TEST(Broadcast, AServerModeIsRequired) {
    auto socket = CAsioBroadcastSocket::create(0, "", "", NextPort());
    std::atomic<int> errors{0};
    socket->errorNotify.connect([&](std::error_code e) {
        if (e == std::errc::invalid_argument)
            errors++;
    });
    socket->initServer(AB_CLIENT);
    EXPECT_EQ(errors, 1);
}

TEST(Broadcast, TheSystemTargetsAreBroadcastCapableInterfaces) {
    for (const auto& t : CAsioBroadcastSocket::systemTargets()) {
        std::error_code ec;
        auto address = asio::ip::make_address_v4(t.address, ec);
        EXPECT_FALSE(ec) << t.address;
        EXPECT_FALSE(address.is_loopback()) << t.address;
        asio::ip::make_address_v4(t.destination, ec);
        EXPECT_FALSE(ec) << t.destination;
    }
}
