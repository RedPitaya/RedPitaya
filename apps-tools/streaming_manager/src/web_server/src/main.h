#pragma once

#include <sys/syslog.h>  //Add custom RP_LCR LOG system

#include <CustomParameters.h>
#include <DataManager.h>

#include <complex.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <ctime>
#include <fstream>

#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

#include <signal.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// The entry points of the page backend; streaming-server calls them (web_page.cpp)
int rp_app_init(void);
int rp_app_exit(void);
// In streaming-server (the service of the page): the options of the session, before rp_app_init.
// Without a call the module keeps its own: the default config file, every interface.
void rp_app_set_session_options(const char* configFile, const char* listenHost, bool verbose);

auto isDACAviable() -> bool;

#ifdef __cplusplus
}
#endif
