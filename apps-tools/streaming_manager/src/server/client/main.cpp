#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <system_error>
#include <functional>
#include <asio.hpp>
#include <chrono>

#include "options.h"
#include "search.h"
#include "config.h"
#include "remote.h"
#include "streaming.h"
#include "dac_streaming.h"


using namespace std;

const char *g_argv0 = NULL;

void sigHandler (int){
    remoteSIGHandler();
    configSIGHandler();
    streamingSIGHandler();
}

void installTermSignalHandler()
{
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

int main(int argc, char* argv[])
{
    installTermSignalHandler();

    g_argv0 = argv[0];
    auto opt = ClientOpt::parse(argc,argv);
    if (opt.mode == ClientOpt::Mode::ERROR_MODE) {
        ClientOpt::usage(g_argv0);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::SEARCH) {
        startSearch(opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::CONFIG){
        startConfig(opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::REMOTE){
        startRemote(opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::STREAMING){
        startStreaming(opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::STREAMING_DAC){
        startDACStreaming(opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::STREAMING_DAC_CONF){
        startDACStreaming(opt.streaming_conf_file);
        return 0;
    }

    return 0;
}
