#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>
#include "broadcast_lib/asio_broadcast_socket.h"
#include "logger_lib/file_logger.h"

#ifndef _WIN32
#include <syslog.h>
#define SYS(X, ...) syslog(X, __VA_ARGS__);
#else
#define SYS(X, ...)
#endif

#define printWithLog(X, Y, ...) \
    aprintf(Y, __VA_ARGS__);    \
    SYS(X, __VA_ARGS__);

namespace ClientOpt {

struct Options {
    bool background;
    std::string conf_file;
    std::string listen;  // the config and data ports: empty - every interface
    bool verbose;
    // the web page: a WebSocket on 127.0.0.1 that nginx proxies to the page; 0 - none
    uint16_t webPort;

    Options() {
        verbose = false;
#ifdef STREAMING_WEB
        webPort = 18906;
#else
        webPort = 0;
#endif
        background = false;
#ifdef RP_PLATFORM
        conf_file = std::string("/root/.config/redpitaya/apps/streaming/streaming_config.json");
#else
        conf_file = std::string("./server.conf");
#endif
    };
};
auto split(const std::string& s, char seperator) -> std::vector<std::string>;
auto usage(char const* progName) -> void;
auto parse(int argc, char* argv[]) -> Options;

}  // namespace ClientOpt

#endif
