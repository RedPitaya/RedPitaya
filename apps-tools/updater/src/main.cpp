#include <CustomParameters.h>
#include <DataManager.h>

#include <unistd.h>
#include <atomic>
#include <chrono>
#include <thread>

#include "common/rp_updater.h"
#include "common/version.h"
#include "main.h"

// 5 min
#define SLEEP_TIME 1000 * 5

using namespace std::chrono;

std::atomic_bool g_stop = false;
std::thread* g_updaterReqThread = nullptr;

CStringParameter g_last_release("RP_LAST_RELEASE", CBaseParameter::RO, "", 250);

const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya update manager.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading updater %s-%s.\n", VERSION_STR, REVISION_STR);
#ifdef ZIP_DISABLED
    CDataManager::GetInstance()->SetEnableParamsGZip(false);
    CDataManager::GetInstance()->SetEnableSignalsGZip(false);
    CDataManager::GetInstance()->SetEnableBinarySignalsGZip(false);
#endif
    CDataManager::GetInstance()->SetParamInterval(1000);
    CDataManager::GetInstance()->SetSignalInterval(1000);

    g_updaterReqThread = new std::thread([]() {
        g_stop = false;
        auto now = system_clock::now();
        auto curTime = time_point_cast<milliseconds>(now);
        auto lastTime = curTime;
        int64_t sleep_time = 0;
        while (true) {
            curTime = time_point_cast<milliseconds>(system_clock::now());
            if ((curTime - lastTime).count() > sleep_time) {
                std::vector<std::string> files;
                if (rp_UpdaterGetReleaseAvailableFilesList(files) == RP_UP_OK) {
                    if (files.size()) {
                        g_last_release.SendValue(files.back());
                    }
                }
                sleep_time = SLEEP_TIME;
                lastTime = curTime;
            }
            usleep(300000);
            if (g_stop) {
                break;
            }
        }
    });
    return 0;
}

int rp_app_exit(void) {
    if (g_updaterReqThread) {
        if (g_updaterReqThread->joinable()) {
            g_stop = true;
            g_updaterReqThread->join();
        }
    }
    fprintf(stderr, "Unloading updater %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

void UpdateParams(void) {}

void OnNewParams(void) {}

void OnNewSignals(void) {}
