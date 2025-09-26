#include <CustomParameters.h>
#include <DataManager.h>

#include <atomic>
#include <chrono>
#include <thread>

#include <pwd.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>

#include "common/rp_updater.h"
#include "common/version.h"
#include "main.h"
#include "rp.h"
#include "web/rp_client.h"
#include "web/rp_system.h"

// 5 min
#define SLEEP_TIME 60000 * 5

#define BASE_RATE_PATH getHomeDirectory() + "/.config/redpitaya"
#define ADC_BASE_RATE_PATH getHomeDirectory() + "/.config/redpitaya/adc_base_rate_" + std::to_string((int)getModel()) + ".conf"
#define DAC_BASE_RATE_PATH getHomeDirectory() + "/.config/redpitaya/dac_base_rate_" + std::to_string((int)getModel()) + ".conf"

using namespace std::chrono;

std::atomic_bool g_stop = false;
std::thread* g_updaterReqThread = nullptr;

CStringParameter g_last_release("RP_LAST_RELEASE", CBaseParameter::RO, "", 250);

CIntParameter adc_base_rate("RP_ADC_BASE_RATE", CBaseParameter::RW, 0, 0, 0, INT32_MAX);
CIntParameter dac_base_rate("RP_DAC_BASE_RATE", CBaseParameter::RW, 0, 0, 0, INT32_MAX);

const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya main menu application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading main menu %s-%s.\n", VERSION_STR, REVISION_STR);
    // Need for reset fpga by default
    CDataManager::GetInstance()->SetParamInterval(1000);
    CDataManager::GetInstance()->SetSignalInterval(1000);
    rp_Init();
    rp_WS_Init();
    rp_WS_SetInterval(RP_WS_DISK_SIZE, 10000);
    rp_WS_SetInterval(RP_WS_RAM, 5000);
    rp_WS_SetInterval(RP_WS_SENSOR_VOLT, 2000);
    rp_WS_SetMode((rp_system_mode_t)(RP_WS_ALL & ~RP_WS_SLOW_DAC));
    rp_WS_UpdateParameters(true);
    rp_WC_Init();

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
    adc_base_rate.SendValue(rp_HPGetBaseFastADCSpeedHzOrDefault());
    dac_base_rate.SendValue(rp_HPGetBaseFastDACSpeedHzOrDefault());
    return 0;
}

auto isDirectory(const std::string& _path) -> bool {
    struct stat st;

    if (stat(_path.c_str(), &st) == 0) {
        return st.st_mode & S_IFDIR;
    }

    return false;
}

auto createDirectory(const std::string& _path) -> bool {
    size_t pos = 0;

    for (;;) {
        pos = _path.find('/', pos);

        if (pos == std::string::npos) {
            // Create the last directory
            if (!isDirectory(_path.c_str())) {
                int mkdir_err = mkdir(_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                return (mkdir_err == 0) || (mkdir_err == EEXIST);
            } else {
                return true;
            }
        } else {
            ++pos;
            std::string sub_path = _path.substr(0, pos);

            // Create subdirectory
            if (!isDirectory(sub_path.c_str())) {
                int mkdir_err = mkdir(sub_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

                if (!((mkdir_err == 0) || (mkdir_err == EEXIST))) {
                    return false;
                }
            }

            if (pos >= _path.size()) {
                return true;
            }
        }
    }

    return false;
}

auto getModel() -> rp_HPeModels_t {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }
    return c;
}

auto getHomeDirectory() -> std::string {
    // Use getpwuid
    char buf[1024];
    passwd pw;
    passwd* ppw = nullptr;

    if (getpwuid_r(getuid(), &pw, buf, sizeof(buf), &ppw) == 0) {
        return pw.pw_dir;
    }
    return "";
}

int rp_app_exit(void) {
    if (g_updaterReqThread) {
        if (g_updaterReqThread->joinable()) {
            g_stop = true;
            g_updaterReqThread->join();
        }
    }
    rp_Release();
    fprintf(stderr, "Unloading main menu %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

int rp_set_params(rp_app_params_t*, int) {
    return 0;
}

int rp_get_params(rp_app_params_t**) {
    return 0;
}

int rp_get_signals(float***, int*, int*) {
    return 0;
}

void UpdateParams(void) {
    rp_WS_UpdateParameters(false);
}

void PostUpdateSignals(void) {}

void UpdateSignals(void) {}

void OnNewParams(void) {
    auto adc_rate_path = ADC_BASE_RATE_PATH;
    auto dac_rate_path = DAC_BASE_RATE_PATH;
    if (adc_base_rate.NewValue() != adc_base_rate.Value()) {
        adc_base_rate.Update();
        createDirectory(BASE_RATE_PATH);
        if (adc_base_rate.Value() > 0) {
            std::ofstream out;
            out.open(adc_rate_path);
            if (out.is_open()) {
                out << adc_base_rate.Value();
            }
            out.close();
        }
        rp_HPInit();
        adc_base_rate.SendValue(rp_HPGetBaseFastADCSpeedHzOrDefault());
    }

    if (dac_base_rate.NewValue() != dac_base_rate.Value()) {
        dac_base_rate.Update();
        createDirectory(BASE_RATE_PATH);
        if (dac_base_rate.Value() > 0) {
            std::ofstream out;
            out.open(dac_rate_path);
            if (out.is_open()) {
                out << dac_base_rate.Value();
            }
            out.close();
        }
        rp_HPInit();
        dac_base_rate.SendValue(rp_HPGetBaseFastDACSpeedHzOrDefault());
    }
}

void OnNewSignals(void) {}
