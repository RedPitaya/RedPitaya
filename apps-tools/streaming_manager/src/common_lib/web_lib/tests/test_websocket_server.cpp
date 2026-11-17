// web_lib::CWebSocketServer against a raw TCP client that builds the frames by hand: the
// handshake, masked frames of every length form, fragments, ping, close, protocol errors, limits,
// several clients and the stop.

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "asio.hpp"
#include "web_lib/websocket_server.h"

using web_lib::CWebSocketServer;
using namespace std::chrono_literals;

namespace {

// Waits for a success only, returning at once on it: the limit is for the slow machines (the
// emulated ARM of CI, the sanitizers), not for the time a test takes.
auto WaitFor(const std::function<bool()>& done, std::chrono::milliseconds timeout = 20000ms) -> bool {
    const auto end = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < end) {
        if (done())
            return true;
        std::this_thread::sleep_for(2ms);
    }
    return done();
}

struct Frame {
    bool fin = true;
    uint8_t op = 0;
    std::string payload;
};

// A WebSocket client over a blocking socket.
class Client {
   public:
    explicit Client(uint16_t port) : m_socket(m_io) {
        m_socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));
    }

    auto raw(const std::string& bytes) -> void { asio::write(m_socket, asio::buffer(bytes)); }

    // The HTTP answer to an upgrade request.
    auto upgrade(const std::string& request) -> std::string {
        raw(request);
        std::string head;
        char c;
        while (head.find("\r\n\r\n") == std::string::npos) {
            asio::error_code ec;
            if (asio::read(m_socket, asio::buffer(&c, 1), ec) != 1)
                break;
            head += c;
        }
        return head;
    }

    auto handshake() -> bool {
        const auto r = upgrade(
            "GET /streaming_ws HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n");
        return r.rfind("HTTP/1.1 101", 0) == 0 && r.find("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") != std::string::npos;
    }

    // A masked frame, as a client sends it.
    static auto frame(uint8_t op, const std::string& payload, bool fin = true, bool masked = true) -> std::string {
        std::string f;
        f += char((fin ? 0x80 : 0) | op);
        const uint8_t m = masked ? 0x80 : 0;
        const size_t n = payload.size();
        if (n < 126) {
            f += char(m | n);
        } else if (n <= 0xFFFF) {
            f += char(m | 126);
            f += char(n >> 8);
            f += char(n & 0xFF);
        } else {
            f += char(m | 127);
            for (int i = 7; i >= 0; i--)
                f += char((uint64_t(n) >> (i * 8)) & 0xFF);
        }
        const char key[4] = {0x12, 0x34, 0x56, 0x78};
        if (masked)
            f.append(key, 4);
        for (size_t i = 0; i < n; i++)
            f += masked ? char(payload[i] ^ key[i % 4]) : payload[i];
        return f;
    }

    auto send(uint8_t op, const std::string& payload, bool fin = true) -> void { raw(frame(op, payload, fin)); }

    // The next frame from the server; none when the connection closes.
    auto read() -> std::optional<Frame> {
        uint8_t h[2];
        asio::error_code ec;
        if (asio::read(m_socket, asio::buffer(h, 2), ec) != 2)
            return std::nullopt;
        EXPECT_FALSE(h[1] & 0x80) << "a server never masks";
        uint64_t len = h[1] & 0x7F;
        if (len == 126) {
            uint8_t e[2];
            asio::read(m_socket, asio::buffer(e, 2), ec);
            len = uint64_t(e[0]) << 8 | e[1];
        } else if (len == 127) {
            uint8_t e[8];
            asio::read(m_socket, asio::buffer(e, 8), ec);
            len = 0;
            for (auto b : e)
                len = len << 8 | b;
        }
        Frame f;
        f.fin = h[0] & 0x80;
        f.op = h[0] & 0x0F;
        f.payload.resize(len);
        if (len && asio::read(m_socket, asio::buffer(f.payload), ec) != len)
            return std::nullopt;
        return f;
    }

    // True when the server closes the connection (after a close frame or not).
    auto closedByServer() -> bool {
        for (int i = 0; i < 100; i++) {
            auto f = read();
            if (!f)
                return true;
            if (f->op == 0x8)
                continue;
        }
        return false;
    }

   private:
    asio::io_context m_io;
    asio::ip::tcp::socket m_socket;
};

// A server that echoes every message and counts the opens and closes.
struct Echo {
    CWebSocketServer server;
    std::atomic<int> opened{0};
    std::atomic<int> closed{0};
    std::mutex mtx;
    std::vector<std::string> messages;

    explicit Echo(CWebSocketServer::Limits limits = {}) : server(limits) {
        server.onOpen = [this](CWebSocketServer::ClientId) { opened++; };
        server.onClose = [this](CWebSocketServer::ClientId) { closed++; };
        server.onMessage = [this](CWebSocketServer::ClientId id, const std::string& m) {
            {
                std::lock_guard lock(mtx);
                messages.push_back(m);
            }
            server.send(id, m);
        };
        EXPECT_TRUE(server.start("127.0.0.1", 0));
    }
};

}  // namespace

TEST(WebSocket, TheAcceptKeyIsTheOneOfTheRfc) {
    EXPECT_EQ(web_lib::webSocketAccept("dGhlIHNhbXBsZSBub25jZQ=="), "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

TEST(WebSocket, AMessageGoesThereAndBack) {
    Echo e;
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    ASSERT_TRUE(WaitFor([&] { return e.server.clients() == 1; }));
    c.send(0x1, R"({"parameters":{"SS_RATE":{"value":64}}})");
    auto f = c.read();
    ASSERT_TRUE(f);
    EXPECT_EQ(f->op, 0x1);
    EXPECT_EQ(f->payload, R"({"parameters":{"SS_RATE":{"value":64}}})");
}

TEST(WebSocket, EveryLengthFormArrivesWhole) {
    Echo e;
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    std::mt19937 rng(1);
    for (size_t n : {0u, 1u, 125u, 126u, 65535u, 65536u, 300000u}) {
        std::string m(n, '\0');
        for (auto& ch : m)
            ch = char('a' + rng() % 26);
        c.send(0x1, m);
        auto f = c.read();
        ASSERT_TRUE(f) << n;
        EXPECT_EQ(f->payload, m) << n;
    }
}

TEST(WebSocket, FragmentsMakeOneMessage) {
    Echo e;
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    c.send(0x1, "abc", false);
    c.send(0x9, "between");  // a control frame between fragments is allowed
    c.send(0x0, "def", false);
    c.send(0x0, "ghi", true);
    auto pong = c.read();
    ASSERT_TRUE(pong);
    EXPECT_EQ(pong->op, 0xA);
    EXPECT_EQ(pong->payload, "between");
    auto f = c.read();
    ASSERT_TRUE(f);
    EXPECT_EQ(f->payload, "abcdefghi");
}

TEST(WebSocket, SeveralFramesInOneSegmentAreAllTaken) {
    Echo e;
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    c.raw(Client::frame(0x1, "one") + Client::frame(0x1, "two") + Client::frame(0x1, "three"));
    for (const char* m : {"one", "two", "three"}) {
        auto f = c.read();
        ASSERT_TRUE(f);
        EXPECT_EQ(f->payload, m);
    }
}

TEST(WebSocket, ACloseIsAnsweredAndReported) {
    Echo e;
    {
        Client c(e.server.port());
        ASSERT_TRUE(c.handshake());
        ASSERT_TRUE(WaitFor([&] { return e.opened == 1; }));
        c.send(0x8, std::string("\x03\xE8", 2));
        auto f = c.read();
        ASSERT_TRUE(f);
        EXPECT_EQ(f->op, 0x8);
        EXPECT_EQ(f->payload, std::string("\x03\xE8", 2));
        EXPECT_FALSE(c.read());
    }
    EXPECT_TRUE(WaitFor([&] { return e.closed == 1 && e.server.clients() == 0; }));
}

TEST(WebSocket, ADroppedConnectionIsReported) {
    Echo e;
    {
        Client c(e.server.port());
        ASSERT_TRUE(c.handshake());
        ASSERT_TRUE(WaitFor([&] { return e.opened == 1; }));
    }
    EXPECT_TRUE(WaitFor([&] { return e.closed == 1 && e.server.clients() == 0; }));
}

TEST(WebSocket, AnUnmaskedFrameEndsTheConnection) {
    Echo e;
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    c.raw(Client::frame(0x1, "hello", true, false));
    EXPECT_TRUE(c.closedByServer());
    EXPECT_TRUE(WaitFor([&] { return e.closed == 1; }));
    std::lock_guard lock(e.mtx);
    EXPECT_TRUE(e.messages.empty());
}

TEST(WebSocket, AContinuationWithoutAStartEndsTheConnection) {
    Echo e;
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    c.send(0x0, "orphan");
    EXPECT_TRUE(c.closedByServer());
}

TEST(WebSocket, AMessageOverTheLimitEndsTheConnection) {
    CWebSocketServer::Limits limits;
    limits.maxMessage = 1000;
    Echo e(limits);
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    c.send(0x1, std::string(600, 'x'), false);
    c.send(0x0, std::string(600, 'x'), true);
    EXPECT_TRUE(c.closedByServer());
    std::lock_guard lock(e.mtx);
    EXPECT_TRUE(e.messages.empty());
}

TEST(WebSocket, AnythingButAnUpgradeIsRefused) {
    Echo e;
    Client c(e.server.port());
    const auto r = c.upgrade("GET /streaming_ws HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_EQ(r.rfind("HTTP/1.1 400", 0), 0u) << r;
    EXPECT_EQ(e.opened, 0);
}

TEST(WebSocket, AHeaderThatNeverEndsIsRefused) {
    CWebSocketServer::Limits limits;
    limits.maxHeader = 512;
    Echo e(limits);
    Client c(e.server.port());
    const auto r = c.upgrade("GET / HTTP/1.1\r\nX: " + std::string(2000, 'a'));
    EXPECT_EQ(r.rfind("HTTP/1.1 431", 0), 0u) << r;
}

TEST(WebSocket, ABroadcastReachesEveryClient) {
    Echo e;
    Client a(e.server.port()), b(e.server.port());
    ASSERT_TRUE(a.handshake());
    ASSERT_TRUE(b.handshake());
    ASSERT_TRUE(WaitFor([&] { return e.server.clients() == 2; }));
    e.server.broadcast("to all");
    for (Client* c : {&a, &b}) {
        auto f = c->read();
        ASSERT_TRUE(f);
        EXPECT_EQ(f->payload, "to all");
    }
}

TEST(WebSocket, BinaryMessagesGoOutAsBinaryFrames) {
    Echo e;
    Client c(e.server.port());
    ASSERT_TRUE(c.handshake());
    ASSERT_TRUE(WaitFor([&] { return e.server.clients() == 1; }));
    const std::string data("NZIA\x00\xff\x80{}", 10);
    e.server.broadcast(data, true);
    auto f = c.read();
    ASSERT_TRUE(f);
    EXPECT_EQ(f->op, 0x2);
    EXPECT_EQ(f->payload, data);
}

TEST(WebSocket, ClientsOverTheLimitAreRefused) {
    CWebSocketServer::Limits limits;
    limits.maxClients = 1;
    Echo e(limits);
    Client a(e.server.port());
    ASSERT_TRUE(a.handshake());
    ASSERT_TRUE(WaitFor([&] { return e.server.clients() == 1; }));
    Client b(e.server.port());
    EXPECT_FALSE(b.handshake());
    EXPECT_EQ(e.opened, 1);
}

TEST(WebSocket, ASecondServerOnTheSamePortFails) {
    Echo e;
    CWebSocketServer other;
    EXPECT_FALSE(other.start("127.0.0.1", e.server.port()));
    CWebSocketServer bad;
    EXPECT_FALSE(bad.start("not an address", 0));
}

// The owner frees what the callbacks use right after stop(): none may run after it.
TEST(WebSocket, NoCallbackRunsAfterTheStop) {
    std::atomic<bool> stopped{false};
    std::atomic<bool> late{false};
    std::atomic<int> inside{0};
    CWebSocketServer server;
    server.onMessage = [&](CWebSocketServer::ClientId id, const std::string& m) {
        inside++;
        std::this_thread::sleep_for(50ms);  // the stop comes in the middle
        if (stopped)
            late = true;
        server.send(id, m);
    };
    server.onClose = [&](CWebSocketServer::ClientId) {
        if (stopped)
            late = true;
    };
    ASSERT_TRUE(server.start("127.0.0.1", 0));
    Client c(server.port());
    ASSERT_TRUE(c.handshake());
    for (int i = 0; i < 20; i++)
        c.send(0x1, "flood");
    ASSERT_TRUE(WaitFor([&] { return inside > 0; }));
    server.stop();
    stopped = true;
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(late);
    EXPECT_TRUE(c.closedByServer());
}

TEST(WebSocket, AStopFromACallbackDoesNotHang) {
    auto server = std::make_shared<CWebSocketServer>();
    std::atomic<bool> done{false};
    server->onMessage = [&](CWebSocketServer::ClientId, const std::string&) {
        server->stop();
        done = true;
    };
    ASSERT_TRUE(server->start("127.0.0.1", 0));
    Client c(server->port());
    ASSERT_TRUE(c.handshake());
    c.send(0x1, "stop");
    EXPECT_TRUE(WaitFor([&] { return done.load(); }));
}

TEST(WebSocket, AClientThatDoesNotReadIsClosed) {
    CWebSocketServer::Limits limits;
    limits.maxQueued = 256 * 1024;
    CWebSocketServer server(limits);
    std::atomic<int> closed{0};
    server.onClose = [&](CWebSocketServer::ClientId) { closed++; };
    ASSERT_TRUE(server.start("127.0.0.1", 0));
    Client c(server.port());
    ASSERT_TRUE(c.handshake());
    ASSERT_TRUE(WaitFor([&] { return server.clients() == 1; }));
    const std::string big(64 * 1024, 'x');
    // the socket buffers take some; the queue then overflows
    for (int i = 0; i < 400 && closed == 0; i++) {
        server.broadcast(big);
        std::this_thread::sleep_for(1ms);
    }
    EXPECT_TRUE(WaitFor([&] { return closed == 1; }));
}
