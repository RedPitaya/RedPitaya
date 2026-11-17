#include "websocket_server.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <deque>
#include <map>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "asio.hpp"
#include "logger_lib/file_logger.h"

namespace web_lib {

namespace {

// --- SHA-1 and base64, only for the handshake -------------------------------------------------

auto rol(uint32_t v, int n) -> uint32_t {
    return (v << n) | (v >> (32 - n));
}

auto sha1(const std::string& data) -> std::array<uint8_t, 20> {
    uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    std::vector<uint8_t> m(data.begin(), data.end());
    const uint64_t bits = uint64_t(data.size()) * 8;
    m.push_back(0x80);
    while (m.size() % 64 != 56)
        m.push_back(0);
    for (int i = 7; i >= 0; i--)
        m.push_back(uint8_t(bits >> (i * 8)));
    for (size_t off = 0; off < m.size(); off += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; i++)
            w[i] = uint32_t(m[off + i * 4]) << 24 | uint32_t(m[off + i * 4 + 1]) << 16 | uint32_t(m[off + i * 4 + 2]) << 8 | m[off + i * 4 + 3];
        for (int i = 16; i < 80; i++)
            w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            const uint32_t t = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = t;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }
    std::array<uint8_t, 20> out{};
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 4; j++)
            out[i * 4 + j] = uint8_t(h[i] >> (24 - j * 8));
    return out;
}

auto base64(const uint8_t* p, size_t n) -> std::string {
    static const char* t = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    for (size_t i = 0; i < n; i += 3) {
        const uint32_t v = uint32_t(p[i]) << 16 | (i + 1 < n ? uint32_t(p[i + 1]) << 8 : 0) | (i + 2 < n ? p[i + 2] : 0);
        out += t[(v >> 18) & 63];
        out += t[(v >> 12) & 63];
        out += i + 1 < n ? t[(v >> 6) & 63] : '=';
        out += i + 2 < n ? t[v & 63] : '=';
    }
    return out;
}

auto lower(std::string s) -> std::string {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

auto trim(const std::string& s) -> std::string {
    const auto b = s.find_first_not_of(" \t");
    const auto e = s.find_last_not_of(" \t\r");
    return b == std::string::npos ? std::string() : s.substr(b, e - b + 1);
}

// A frame from the server: never masked.
auto frame(uint8_t opcode, const std::string& payload) -> std::string {
    std::string f;
    f += char(0x80 | opcode);
    const size_t n = payload.size();
    if (n < 126) {
        f += char(n);
    } else if (n <= 0xFFFF) {
        f += char(126);
        f += char(n >> 8);
        f += char(n & 0xFF);
    } else {
        f += char(127);
        for (int i = 7; i >= 0; i--)
            f += char((uint64_t(n) >> (i * 8)) & 0xFF);
    }
    return f + payload;
}

constexpr uint8_t OP_CONT = 0x0, OP_TEXT = 0x1, OP_BINARY = 0x2, OP_CLOSE = 0x8, OP_PING = 0x9, OP_PONG = 0xA;

}  // namespace

auto webSocketAccept(const std::string& key) -> std::string {
    const auto d = sha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    return base64(d.data(), d.size());
}

// --- the server ------------------------------------------------------------------------------

// The handlers take the raw this: stop() joins the thread before the owner lets go of it, and a
// stop from a callback leaves the thread holding a reference. A shared_ptr in the handlers made a
// cycle through the io_context (the handlers left in its queue at the stop were never freed).
struct CWebSocketServer::Impl {
    struct Session;
    using SessionPtr = std::shared_ptr<Session>;

    Limits limits;
    std::function<void(ClientId)> onOpen;
    std::function<void(ClientId)> onClose;
    std::function<void(ClientId, const std::string&)> onMessage;

    asio::io_context io;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
    std::thread thread;
    std::atomic<bool> stopping{false};
    std::atomic<uint16_t> boundPort{0};

    // io thread only
    std::map<ClientId, SessionPtr> sessions;
    ClientId nextId = 1;
    std::atomic<size_t> count{0};  // open clients (the handshake done)

    auto recount() -> void {
        size_t n = 0;
        for (auto& [id, s] : sessions)
            n += s->open;
        count = n;
    }

    struct Session : std::enable_shared_from_this<Session> {
        Session(Impl* o, asio::ip::tcp::socket s, ClientId i) : owner(o), socket(std::move(s)), id(i) {}
        Impl* owner;
        asio::ip::tcp::socket socket;
        ClientId id;
        bool open = false;      // the handshake is done and onOpen was called
        bool closing = false;   // a close goes out, then the socket closes
        bool closed = false;
        std::vector<uint8_t> in;
        std::array<uint8_t, 16 * 1024> chunk{};
        std::string message;    // the fragments of the message being received
        uint8_t messageOp = 0;  // 0: none in progress
        std::deque<std::string> out;
        size_t queued = 0;
        bool writing = false;
    };

    auto alive() -> bool { return !stopping; }

    auto accept() -> void {
        acceptor->async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket socket) {
            if (ec || stopping)
                return;
            if (sessions.size() >= limits.maxClients) {
                WARNING("Web socket: %zu clients already, one more is refused", sessions.size())
                asio::error_code ignored;
                socket.close(ignored);
            } else {
                asio::error_code ignored;
                socket.set_option(asio::ip::tcp::no_delay(true), ignored);
                auto s = std::make_shared<Session>(this, std::move(socket), nextId++);
                sessions[s->id] = s;
                read(s);
            }
            accept();
        });
    }

    auto read(const SessionPtr& s) -> void {
        s->socket.async_read_some(asio::buffer(s->chunk), [this, s](const asio::error_code& ec, size_t n) {
            if (stopping || s->closed)
                return;
            if (ec) {
                close(s);
                return;
            }
            s->in.insert(s->in.end(), s->chunk.begin(), s->chunk.begin() + n);
            if (!s->open && !handshake(s))
                return;
            if (s->open && !frames(s))
                return;
            if (!s->closed && !s->closing)
                read(s);
        });
    }

    // False when the session ends (refused or closed).
    auto handshake(const SessionPtr& s) -> bool {
        const std::string text(s->in.begin(), s->in.end());
        const auto end = text.find("\r\n\r\n");
        if (end == std::string::npos) {
            if (s->in.size() > limits.maxHeader) {
                refuse(s, "431 Request Header Fields Too Large");
                return false;
            }
            return true;  // read on
        }
        std::map<std::string, std::string> headers;
        size_t pos = text.find("\r\n");
        const std::string requestLine = text.substr(0, pos);
        while (pos < end) {
            const auto next = text.find("\r\n", pos + 2);
            const std::string line = text.substr(pos + 2, next - pos - 2);
            const auto colon = line.find(':');
            if (colon != std::string::npos)
                headers[lower(trim(line.substr(0, colon)))] = trim(line.substr(colon + 1));
            pos = next;
        }
        const bool get = requestLine.rfind("GET ", 0) == 0;
        const bool upgrade = lower(headers["upgrade"]) == "websocket" && lower(headers["connection"]).find("upgrade") != std::string::npos;
        const std::string key = headers["sec-websocket-key"];
        if (!get || !upgrade || key.empty() || headers["sec-websocket-version"] != "13") {
            refuse(s, "400 Bad Request");
            return false;
        }
        s->in.erase(s->in.begin(), s->in.begin() + end + 4);
        s->open = true;
        recount();
        enqueue(s, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " + webSocketAccept(key) + "\r\n\r\n");
        if (onOpen && alive())
            onOpen(s->id);
        return !s->closed;
    }

    auto refuse(const SessionPtr& s, const std::string& status) -> void {
        enqueue(s, "HTTP/1.1 " + status + "\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
        s->closing = true;
    }

    // Takes the whole frames out of s->in. False when the session ends.
    auto frames(const SessionPtr& s) -> bool {
        for (;;) {
            auto& b = s->in;
            if (b.size() < 2)
                return true;
            const bool fin = b[0] & 0x80;
            const uint8_t op = b[0] & 0x0F;
            const bool masked = b[1] & 0x80;
            uint64_t len = b[1] & 0x7F;
            size_t head = 2;
            if (len == 126) {
                if (b.size() < 4)
                    return true;
                len = uint64_t(b[2]) << 8 | b[3];
                head = 4;
            } else if (len == 127) {
                if (b.size() < 10)
                    return true;
                len = 0;
                for (int i = 0; i < 8; i++)
                    len = len << 8 | b[2 + i];
                head = 10;
            }
            // a client masks every frame (RFC 6455 5.1); no extensions, so no RSV bits
            if (!masked || (b[0] & 0x70)) {
                failClose(s, 1002);
                return false;
            }
            const bool control = op & 0x08;
            if (control && (!fin || len > 125)) {
                failClose(s, 1002);
                return false;
            }
            if (!control && s->message.size() + len > limits.maxMessage) {
                failClose(s, 1009);
                return false;
            }
            if (b.size() < head + 4 + len)
                return true;
            const uint8_t* mask = &b[head];
            std::string payload(len, '\0');
            for (uint64_t i = 0; i < len; i++)
                payload[i] = char(b[head + 4 + i] ^ mask[i % 4]);
            b.erase(b.begin(), b.begin() + head + 4 + len);

            switch (op) {
                case OP_PING:
                    enqueue(s, frame(OP_PONG, payload));
                    if (s->closed)
                        return false;
                    break;
                case OP_PONG:
                    break;
                case OP_CLOSE:
                    enqueue(s, frame(OP_CLOSE, payload.substr(0, 2)));
                    s->closing = true;
                    return false;
                case OP_TEXT:
                case OP_BINARY:
                    if (s->messageOp) {
                        failClose(s, 1002);
                        return false;
                    }
                    s->messageOp = op;
                    s->message = std::move(payload);
                    break;
                case OP_CONT:
                    if (!s->messageOp) {
                        failClose(s, 1002);
                        return false;
                    }
                    s->message += payload;
                    break;
                default:
                    failClose(s, 1002);
                    return false;
            }
            if (!control && fin) {
                std::string m;
                m.swap(s->message);
                s->messageOp = 0;
                if (onMessage && alive())
                    onMessage(s->id, m);
                if (s->closed || s->closing)
                    return false;
            }
        }
    }

    auto failClose(const SessionPtr& s, uint16_t code) -> void {
        std::string p;
        p += char(code >> 8);
        p += char(code & 0xFF);
        enqueue(s, frame(OP_CLOSE, p));
        s->closing = true;
    }

    auto enqueue(const SessionPtr& s, std::string bytes) -> void {
        if (s->closed)
            return;
        s->queued += bytes.size();
        if (s->queued > limits.maxQueued) {
            WARNING("Web socket: client %llu does not read, it is closed", (unsigned long long)s->id)
            close(s);
            return;
        }
        s->out.push_back(std::move(bytes));
        write(s);
    }

    auto write(const SessionPtr& s) -> void {
        if (s->writing || s->closed)
            return;
        if (s->out.empty()) {
            if (s->closing)
                close(s);
            return;
        }
        s->writing = true;
        asio::async_write(s->socket, asio::buffer(s->out.front()), [this, s](const asio::error_code& ec, size_t) {
            s->writing = false;
            if (stopping || s->closed)
                return;
            if (ec) {
                close(s);
                return;
            }
            s->queued -= s->out.front().size();
            s->out.pop_front();
            write(s);
        });
    }

    auto close(const SessionPtr& s) -> void {
        if (s->closed)
            return;
        s->closed = true;
        asio::error_code ignored;
        s->socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
        s->socket.close(ignored);
        sessions.erase(s->id);
        recount();
        if (s->open && onClose && alive())
            onClose(s->id);
    }

    auto sendData(ClientId id, std::string text, uint8_t op) -> void {
        asio::post(io, [this, id, op, text = std::move(text)]() mutable {
            if (stopping)
                return;
            auto it = sessions.find(id);
            if (it != sessions.end() && it->second->open && !it->second->closing)
                enqueue(it->second, frame(op, text));
        });
    }

    auto broadcastData(const std::string& text, uint8_t op) -> void {
        asio::post(io, [this, f = frame(op, text)]() {
            if (stopping)
                return;
            // enqueue may close a session (and erase it): walk a copy
            std::vector<SessionPtr> all;
            for (auto& [id, s] : sessions)
                all.push_back(s);
            for (auto& s : all)
                if (s->open && !s->closing)
                    enqueue(s, f);
        });
    }
};

CWebSocketServer::CWebSocketServer() : CWebSocketServer(Limits{}) {}

CWebSocketServer::CWebSocketServer(Limits limits) : m_impl(std::make_shared<Impl>()) {
    m_impl->limits = limits;
}

CWebSocketServer::~CWebSocketServer() {
    stop();
}

auto CWebSocketServer::start(const std::string& host, uint16_t port) -> bool {
    auto& p = *m_impl;
    if (p.thread.joinable())
        return false;
    p.onOpen = onOpen;
    p.onClose = onClose;
    p.onMessage = onMessage;
    try {
        const auto address = asio::ip::make_address(host.empty() ? "127.0.0.1" : host);
        asio::ip::tcp::endpoint endpoint(address, port);
        p.acceptor = std::make_unique<asio::ip::tcp::acceptor>(p.io);
        p.acceptor->open(endpoint.protocol());
        p.acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
        p.acceptor->bind(endpoint);
        p.acceptor->listen();
        p.boundPort = p.acceptor->local_endpoint().port();
    } catch (std::exception& e) {
        ERROR_LOG("Web socket: can't listen on %s:%u: %s", host.c_str(), (unsigned)port, e.what())
        p.acceptor.reset();
        return false;
    }
    p.work.emplace(asio::make_work_guard(p.io));
    p.accept();
    auto impl = m_impl;
    p.thread = std::thread([impl]() {
        try {
            impl->io.run();
        } catch (std::exception& e) {
            ERROR_LOG("Web socket: %s", e.what())
        }
    });
    return true;
}

auto CWebSocketServer::stop() -> void {
    auto& p = *m_impl;
    if (!p.thread.joinable())
        return;
    p.stopping = true;
    p.work.reset();
    p.io.stop();
    if (p.thread.get_id() == std::this_thread::get_id()) {
        // from a callback: the callbacks already see `stopping`; the thread ends on its own
        p.thread.detach();
        return;
    }
    p.thread.join();
    for (auto& [id, s] : p.sessions) {
        asio::error_code ignored;
        s->socket.close(ignored);
    }
    p.sessions.clear();
    p.count = 0;
    if (p.acceptor) {
        asio::error_code ignored;
        p.acceptor->close(ignored);
    }
}

auto CWebSocketServer::port() const -> uint16_t {
    return m_impl->boundPort;
}

auto CWebSocketServer::clients() const -> size_t {
    return m_impl->count;
}

auto CWebSocketServer::send(ClientId client, std::string data, bool binary) -> void {
    m_impl->sendData(client, std::move(data), binary ? OP_BINARY : OP_TEXT);
}

auto CWebSocketServer::broadcast(const std::string& data, bool binary) -> void {
    m_impl->broadcastData(data, binary ? OP_BINARY : OP_TEXT);
}

}  // namespace web_lib
