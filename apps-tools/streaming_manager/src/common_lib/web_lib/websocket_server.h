#ifndef WEB_LIB_WEBSOCKET_SERVER_H
#define WEB_LIB_WEBSOCKET_SERVER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace web_lib {

// A small WebSocket server (RFC 6455) for the web page of the board: nginx proxies the page to it
// on the loopback address. Text and binary messages of any size up to a limit, fragmented or not,
// ping/pong and close; no extensions (no compression), no TLS (nginx has it).
//
// Every callback runs on the server's own thread, one at a time. send() and broadcast() may be
// called from any thread, also from a callback. Once stop() returns no callback runs any more.
class CWebSocketServer {
   public:
    using Ptr = std::shared_ptr<CWebSocketServer>;
    using ClientId = uint64_t;

    struct Limits {
        size_t maxClients = 8;
        size_t maxHeader = 8 * 1024;               // the HTTP upgrade request
        size_t maxMessage = 4 * 1024 * 1024;       // a whole message, all its fragments
        size_t maxQueued = 16 * 1024 * 1024;       // bytes waiting to be sent to one client
    };

    CWebSocketServer();
    explicit CWebSocketServer(Limits limits);
    ~CWebSocketServer();

    CWebSocketServer(const CWebSocketServer&) = delete;
    CWebSocketServer& operator=(const CWebSocketServer&) = delete;

    // Set before start.
    std::function<void(ClientId)> onOpen;
    std::function<void(ClientId)> onClose;
    std::function<void(ClientId, const std::string&)> onMessage;

    // Listens on host:port (port 0: any free one, see port()). False if the address is taken or
    // wrong; the reason is logged.
    auto start(const std::string& host, uint16_t port) -> bool;
    // Closes every connection and joins the thread. Not from a callback (it cannot join itself:
    // then the thread is left to end on its own and the callbacks stop at once).
    auto stop() -> void;

    auto port() const -> uint16_t;
    auto clients() const -> size_t;

    // A message to one client or to all, text (UTF-8) or binary. A client whose queue grows past
    // maxQueued (a page that does not read) is closed.
    auto send(ClientId client, std::string data, bool binary = false) -> void;
    auto broadcast(const std::string& data, bool binary = false) -> void;

   private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

// Exposed for the tests.
auto webSocketAccept(const std::string& key) -> std::string;

}  // namespace web_lib

#endif
