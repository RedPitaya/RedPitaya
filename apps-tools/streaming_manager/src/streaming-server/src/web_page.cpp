#include "web_page.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <rp_json.h>

#include "logger_lib/file_logger.h"
#include "web_lib/websocket_server.h"

// the module of the page (web_server/src/main.cpp) and rp_sdk, both C entry points
extern "C" {
int rp_app_init(void);
int rp_app_exit(void);
void rp_app_set_session_options(const char* configFile, const char* listenHost, bool verbose);
int ws_set_params(const char* params);
const char* ws_get_params(void);
int ws_get_params_interval(void);
int ws_gzip(int type, const void* in, void* out, size_t* size);
}

// rp_sdk calls these for the signals of a page; the streaming page has none (nginx left them
// unresolved in the module, a program must have them)
void UpdateSignals(void) {}
void UpdateBinarySignals(void) {}
void PostUpdateSignals(void) {}
void PostUpdateBinarySignals(void) {}
void OnNewSignals(void) {}

namespace {

using namespace std::chrono_literals;

// rp_sdk is not thread safe: the messages of the page and the timer take turns, as on the one
// thread of the nginx module
std::mutex g_sdk;
std::unique_ptr<web_lib::CWebSocketServer> g_ws;
std::thread g_timer;
std::atomic<bool> g_run{false};
std::mutex g_wakeMtx;
std::condition_variable g_wake;

// As the nginx module: every interval the parameters that changed go to every page, binary,
// "NZIA" + JSON or "EZIA" + gzip. Nothing is done while no page is open. An idle page still gets
// an empty message every 15 s, or the proxy closes the connection.
auto timerLoop() -> void {
    auto lastSend = std::chrono::steady_clock::now();
    while (g_run) {
        int ms = 0;
        {
            std::lock_guard lock(g_sdk);
            ms = ws_get_params_interval();
        }
        {
            std::unique_lock lock(g_wakeMtx);
            g_wake.wait_for(lock, std::chrono::milliseconds(ms > 0 ? ms : 100));
        }
        if (!g_run || g_ws->clients() == 0)
            continue;
        std::string payload;
        {
            std::lock_guard lock(g_sdk);
            const char* params = ws_get_params();
            const std::string json = params ? params : "";
            if (json.empty() && std::chrono::steady_clock::now() - lastSend < 15s)
                continue;
            std::vector<uint8_t> zipped;
            size_t size = 0;
            if (ws_gzip(0, json.c_str(), &zipped, &size) == 0)
                payload = "EZIA" + std::string(zipped.begin(), zipped.end());
            else
                payload = "NZIA" + json;
        }
        g_ws->broadcast(payload, true);
        lastSend = std::chrono::steady_clock::now();
    }
}

}  // namespace

auto web_page::start(uint16_t port, const std::string& configFile, const std::string& listen, bool verbose) -> bool {
    {
        std::lock_guard lock(g_sdk);
        rp_app_set_session_options(configFile.c_str(), listen.c_str(), verbose);
        rp_app_init();
    }
    g_ws = std::make_unique<web_lib::CWebSocketServer>();
    // As the nginx module: a message of the page is {"parameters": {...}} or {"signals": {...}};
    // rp_sdk takes what is inside "parameters" (the page has no signals)
    g_ws->onMessage = [](web_lib::CWebSocketServer::ClientId, const std::string& message) {
        try {
            std::lock_guard lock(g_sdk);
            JSONNode n = libjson::parse(message);
            if (n.size() == 0)
                return;
            JSONNode child = n.at(0);
            if (child.name() != "parameters")
                return;
            ws_set_params(child.write().c_str());
        } catch (std::exception& e) {
            ERROR_LOG("A message of the web page is not taken: %s", e.what())
            return;
        }
        // the answer goes now, not after the interval
        g_wake.notify_all();
    };
    if (!g_ws->start("127.0.0.1", port)) {
        ERROR_LOG("The web page can't connect: port %u on 127.0.0.1 is taken", (unsigned)port)
        return true;
    }
    g_run = true;
    g_timer = std::thread(timerLoop);
    return true;
}

auto web_page::stop() -> void {
    {
        std::lock_guard lock(g_wakeMtx);
        g_run = false;
    }
    g_wake.notify_all();
    if (g_timer.joinable())
        g_timer.join();
    // no message of a page reaches the module once the WebSocket stopped
    if (g_ws)
        g_ws->stop();
    g_ws.reset();
    std::lock_guard lock(g_sdk);
    rp_app_exit();
}
