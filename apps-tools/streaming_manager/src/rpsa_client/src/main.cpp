#include <signal.h>
#include <sys/types.h>
#include <asio.hpp>

#include "config.h"
#include "config_net_lib/client_net_config_manager.h"
#include "dac_streaming.h"
#include "help.h"
#include "logger_lib/file_logger.h"
#include "options.h"
#include "remote.h"
#include "search.h"
#include "streaming.h"
#include "test_helper.h"

const char* g_argv0 = NULL;

std::shared_ptr<ClientNetConfigManager> g_cl;

void sigHandler(int) {
    remoteSIGHandler();
    configSIGHandler();
    streamingSIGHandler();
    dac_streamingSIGHandler();
}

void installTermSignalHandler() {
#ifdef _WIN32
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
#else
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sigHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
#endif
}

int main(int argc, char* argv[]) {
    using namespace ClientHelp;
    g_argv0 = argv[0];
    const auto prog = programName(argv[0]);
    auto opt = ClientOpt::parse(argc, argv);

    switch (opt.mode) {
        case ClientOpt::Mode::SHOW_HELP:
            return print(stdout, prog, opt.help_topic, opt.help_filter) ? EXIT_OK : EXIT_USAGE;
        case ClientOpt::Mode::SHOW_VERSION:
            printf("%s %s\n", prog.c_str(), version().c_str());
            return EXIT_OK;
        case ClientOpt::Mode::ERROR_MODE:  // no mode given
            print(stderr, prog, "");
            return EXIT_USAGE;
        case ClientOpt::Mode::ERROR_PARAM:  // the error is told already
            synopsis(stderr, prog, opt.help_topic);
            return EXIT_USAGE;
        default:
            break;
    }

    g_cl = std::make_shared<ClientNetConfigManager>("", false);
    installTermSignalHandler();
    setOptions(opt);
    setReplyTimeout(opt.reply_timeout_ms);

    if (opt.mode == ClientOpt::Mode::SEARCH)
        return startSearch(opt) ? EXIT_OK : EXIT_NO_BOARD;

    if (opt.hosts.size() == 0) {
        auto host = startSearch();
        if (host == "") {
            aprintf(stderr, "[ERROR] No board found: none answered the broadcast in 1 s. Give the board with -h IP\n");
            return EXIT_NO_BOARD;
        }
        opt.hosts.push_back(host);
    }

    if (!connectConfigServer(g_cl, opt)) {
        g_cl = nullptr;
        return EXIT_NO_BOARD;
    }

    bool ok = true;
    if (opt.mode == ClientOpt::Mode::CONFIG)
        ok = startConfig(g_cl, opt);
    if (opt.mode == ClientOpt::Mode::REMOTE)
        ok = startRemote(g_cl, opt);
    if (opt.mode == ClientOpt::Mode::STREAMING)
        ok = startStreaming(g_cl, opt);
    if (opt.mode == ClientOpt::Mode::STREAMING_DAC)
        ok = startDACStreaming(g_cl, opt);

    g_cl = nullptr;
    return ok ? EXIT_OK : EXIT_FAILED;
}
