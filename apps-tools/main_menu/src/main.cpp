#include <DataManager.h>
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
#include "web/rp_system.h"
#include "web/rp_client.h"

const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya main menu application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading main menu %s-%s.\n", VERSION_STR, REVISION_STR);
    // Need for reset fpga by default
    CDataManager::GetInstance()->SetParamInterval(1000);
    CDataManager::GetInstance()->SetSignalInterval(1000);
    rp_Init();
    rp_WS_Init();
    rp_WS_SetInterval(RP_WS_DISK_SIZE,10000);
    rp_WS_SetInterval(RP_WS_RAM,5000);
    rp_WS_SetInterval(RP_WS_SENSOR_VOLT,2000);
    rp_WS_SetMode((rp_system_mode_t)(RP_WS_ALL & ~RP_WS_SLOW_DAC));
    rp_WS_UpdateParameters(true);
    rp_WC_Init();
    return 0;
}

int rp_app_exit(void) {
    rp_Release();
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

void UpdateParams(void) {
    rp_WS_UpdateParameters(false);
}

void PostUpdateSignals(void){}

void UpdateSignals(void) {}

void OnNewParams(void) {
}

void OnNewSignals(void){}
