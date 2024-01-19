
#include <string>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "common/version.h"
#include "main.h"
#include "rp.h"

const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya main menu application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading main menu %s-%s.\n", VERSION_STR, REVISION_STR);
    // Need for reset fpga by default
    rp_Init();
    rp_Release();
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading main menu %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

int rp_set_params(rp_app_params_t *, int) {
    return 0;
}

int rp_get_params(rp_app_params_t **) {
    return 0;
}

int rp_get_signals(float ***, int *, int *) {
    return 0;
}



void UpdateParams(void) {}

void PostUpdateSignals(void){}

void UpdateSignals(void) {}

void OnNewParams(void) {}

void OnNewSignals(void){}
