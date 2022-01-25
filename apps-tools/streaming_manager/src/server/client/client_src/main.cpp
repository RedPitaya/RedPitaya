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
#include "test_helper.h"
#include "ClientNetConfigManager.h"


using namespace std;

const char *g_argv0 = NULL;

std::shared_ptr<ClientNetConfigManager> g_cl;

void sigHandler (int){
    remoteSIGHandler();
    configSIGHandler();
    streamingSIGHandler();
    dac_streamingSIGHandler();
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
    g_cl = std::make_shared<ClientNetConfigManager>("",false);
    installTermSignalHandler();

    g_argv0 = argv[0];
    auto opt = ClientOpt::parse(argc,argv);
    setOptions(opt);
    
    if (opt.mode == ClientOpt::Mode::ERROR_MODE) {
        ClientOpt::usage(g_argv0);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::SEARCH) {
        startSearch(opt);
        return 0;
    }

    if (connectConfigServer(g_cl,opt) == false){
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::CONFIG){
        startConfig(g_cl,opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::REMOTE){
        startRemote(g_cl,opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::STREAMING){
        startStreaming(g_cl,opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::STREAMING_DAC){
        startDACStreaming(g_cl,opt);
        return 0;
    }

    if (opt.mode == ClientOpt::Mode::STREAMING_DAC_CONF){
        startDACStreaming(g_cl,opt.streaming_conf_file);
        return 0;
    }
    return 0;
}
