#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>
#include "data_lib/thread_cout.h"

#ifndef _WIN32
#include <syslog.h>
#define SYS(X,...) syslog(X,__VA_ARGS__);
#else
#define SYS(X,...)
#endif

#define printWithLog(X,Y,...) \
    aprintf(Y,__VA_ARGS__); \
    SYS(X,__VA_ARGS__);


namespace ClientOpt {

    struct Options {
        bool background;
        std::string config_port;
        std::string broadcast_port;
        std::string conf_file;
        bool verbose;

        Options(){
            verbose = false;
            background = false;
            config_port = std::string("8901");
            broadcast_port = std::string("8902");
            conf_file = std::string("/root/.streaming_config");
        };
    };
    auto split(const std::string& s, char seperator) ->  std::vector<std::string>;
    auto usage(char const *progName) -> void;
    auto parse(int argc, char* argv[]) -> Options;

}

#endif
