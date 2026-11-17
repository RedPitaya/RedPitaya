#ifndef STREAMING_SERVER_WEB_PAGE_H
#define STREAMING_SERVER_WEB_PAGE_H

#include <cstdint>
#include <string>

// The backend of the streaming web page (the rp_sdk module of the page, web_server/src/main.cpp)
// run inside the server, as nginx ran it before: it owns the streaming session. nginx proxies the
// page to a WebSocket on 127.0.0.1:port. The page reaches the server whatever happens to the tab:
// the streams go on without it.
namespace web_page {

// Starts the session (rp_app_init: a session that cannot start is logged and shown on the page)
// and the WebSocket. A WebSocket that cannot listen is logged: the session runs without a page.
auto start(uint16_t port, const std::string& configFile, const std::string& listen, bool verbose) -> bool;
// Stops the WebSocket, then the session (rp_app_exit).
auto stop() -> void;

}  // namespace web_page

#endif
