#include <CustomParameters.h>
#include <DataManager.h>

#include <atomic>
#include <chrono>
#include <thread>

#include <pwd.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>

#include "common/rp_updater.h"
#include "common/version.h"
#include "main.h"
#include "rp.h"
#include "web/rp_client.h"
#include "web/rp_system.h"
#include "web/rp_websocket.h"

// 5 min
#define SLEEP_TIME 60000 * 5
#define WEBPORT 9099

using namespace std::chrono;

enum RP_COMMDANS { RP_NONE = 0, RP_START_BUILD_LOG = 1, RP_STOP_BUILD_LOG = 2 };

std::atomic_bool g_stop = false;
std::atomic_bool g_stopBuildLog = false;
std::atomic_bool g_isWorkBuildLog = false;
std::thread* g_updaterReqThread = nullptr;
std::thread* g_buildLog = nullptr;

rp_websocket::CWEBServer::Ptr g_server = nullptr;

CStringParameter g_last_release("RP_LAST_RELEASE", CBaseParameter::RO, "", 250);

CIntParameter adc_base_rate("RP_ADC_BASE_RATE", CBaseParameter::RW, 0, 0, 0, INT32_MAX);
CIntParameter dac_base_rate("RP_DAC_BASE_RATE", CBaseParameter::RW, 0, 0, 0, INT32_MAX);
CIntParameter rp_command("RP_COMMAND", CBaseParameter::RW, 0, 0, 0, INT32_MAX);

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

    g_server = std::make_shared<rp_websocket::CWEBServer>();
    g_server->startServer(WEBPORT);

    return 0;
}

auto createDirectory(const std::string& _path) -> bool {
    std::error_code ec;
    return std::filesystem::create_directories(_path, ec);
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
    g_server = nullptr;
    fprintf(stderr, "Unloading main menu %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

void UpdateParams(void) {
    rp_WS_UpdateParameters(false);
}

void OnNewParams(void) {
    if (adc_base_rate.NewValue() != adc_base_rate.Value()) {
        int old = adc_base_rate.Value();
        adc_base_rate.Update();
        if (adc_base_rate.Value() > 0) {
            rp_HPWriteUserDefinedValue("fast_adc_rate", adc_base_rate.Value());
        }
        if (adc_base_rate.Value() == 0) {
            adc_base_rate.SendValue(old);
        }
    }

    if (dac_base_rate.NewValue() != dac_base_rate.Value()) {
        int old = dac_base_rate.Value();
        dac_base_rate.Update();
        if (dac_base_rate.Value() > 0) {
            rp_HPWriteUserDefinedValue("fast_dac_rate", dac_base_rate.Value());
        }
        if (dac_base_rate.Value() == 0) {
            dac_base_rate.SendValue(old);
        }
    }

    if (rp_command.IsNewValue()) {
        rp_command.Update();
        if (rp_command.Value() == RP_START_BUILD_LOG) {
            if (g_isWorkBuildLog == false) {
                g_isWorkBuildLog = true;
                g_buildLog = new std::thread([]() {
                    g_stopBuildLog = false;
                    generateBugReport();
                    g_isWorkBuildLog = false;
                    g_server->send("build_log_done", g_stopBuildLog ? 1 : 0);
                });
            }
        }
        if (rp_command.Value() == RP_STOP_BUILD_LOG) {
            g_stopBuildLog = true;
        }
        rp_command.SendValue(RP_NONE);
    }
}

void generateBugReport() {
    const std::string DEST_FILE = "/tmp/bug_rep_out.zip";

    char tmp_path[] = "/tmp/bug_report_XXXXXX";
    if (!mkdtemp(tmp_path)) {
        perror("Failed to create tmp dir");
        return;
    }
    std::string TEST_TMP_DIR = tmp_path;

    std::vector<std::string> commands = {"dmesg > " + TEST_TMP_DIR + "/dmesg.log",
                                         "journalctl > " + TEST_TMP_DIR + "/journalctl.log",
                                         "systemctl > " + TEST_TMP_DIR + "/systemctl.log",
                                         "systemctl status --all > " + TEST_TMP_DIR + "/systemctl_status.log",
                                         "lsblk > " + TEST_TMP_DIR + "/lsblk.log",
                                         "df -h > " + TEST_TMP_DIR + "/df.log",
                                         "tree /opt -s -D --info > " + TEST_TMP_DIR + "/tree.log 2>/dev/null",
                                         "ifconfig -a > " + TEST_TMP_DIR + "/ifconfig.log 2>/dev/null",
                                         "ip a > " + TEST_TMP_DIR + "/ip_a.log",
                                         "lsusb -t > " + TEST_TMP_DIR + "/usb.log",
                                         "fw_printenv > " + TEST_TMP_DIR + "/fw_printenv.log 2>/dev/null",

                                         "mkdir -p " + TEST_TMP_DIR + "/fpga " + TEST_TMP_DIR + "/ecosystem " + TEST_TMP_DIR + "/logs",

                                         "i2cdetect -y -r 0 > " + TEST_TMP_DIR + "/i2c.log 2>/dev/null",
                                         "monitor -ph > " + TEST_TMP_DIR + "/fpga/reg_house.log",
                                         "monitor -posc > " + TEST_TMP_DIR + "/fpga/reg_osc.log",
                                         "monitor -pasg > " + TEST_TMP_DIR + "/fpga/reg_sig_gen.log",
                                         "monitor -pams > " + TEST_TMP_DIR + "/fpga/reg_ams.log",
                                         "monitor -pdaisy > " + TEST_TMP_DIR + "/fpga/reg_daisy.log",

                                         "cp /tmp/loaded_fpga.inf " + TEST_TMP_DIR + " 2>/dev/null",
                                         "cp /tmp/sysinfo.json " + TEST_TMP_DIR + " 2>/dev/null",
                                         "cp /opt/redpitaya/*.conf " + TEST_TMP_DIR + " 2>/dev/null",
                                         "cp -r /root/.config/redpitaya " + TEST_TMP_DIR + "/ecosystem 2>/dev/null",
                                         "cp /root/.version " + TEST_TMP_DIR + "/OS_Ver.log 2>/dev/null",
                                         "cp -r /var/log/* " + TEST_TMP_DIR + "/logs 2>/dev/null",

                                         "calib -rv > " + TEST_TMP_DIR + "/ecosystem/calib_rv.log 2>/dev/null",
                                         "calib -rvf > " + TEST_TMP_DIR + "/ecosystem/calib_rvf.log 2>/dev/null",
                                         "calib -rvx > " + TEST_TMP_DIR + "/ecosystem/calib_rvx.log 2>/dev/null",
                                         "calib -u > " + TEST_TMP_DIR + "/ecosystem/calib_u.log 2>/dev/null",
                                         "profiles -p > " + TEST_TMP_DIR + "/ecosystem/profiles_p.log 2>/dev/null",
                                         "monitor -ams > " + TEST_TMP_DIR + "/ecosystem/monitor_ams.log",
                                         "monitor -f > " + TEST_TMP_DIR + "/ecosystem/monitor.log",
                                         "monitor -i >> " + TEST_TMP_DIR + "/ecosystem/monitor.log",
                                         "monitor -n >> " + TEST_TMP_DIR + "/ecosystem/monitor.log"};
    uint32_t total = commands.size() + 3;
    uint32_t step = 0;
    g_server->send("build_log_step", static_cast<uint32_t>(step));
    system("rw");
    step++;
    g_server->send("build_log_step", static_cast<uint32_t>((100 * step) / total));
    for (const auto& cmd : commands) {
        system(cmd.c_str());
        step++;
        g_server->send("build_log_step", static_cast<uint32_t>((100 * step) / total));
        if (g_stopBuildLog) {
            break;
        }
    }
    if (!g_stopBuildLog) {
        std::string zip_cmd = "cd " + TEST_TMP_DIR + " && zip -9 -r " + DEST_FILE + " . * && cd -";
        system(zip_cmd.c_str());
        step++;
        g_server->send("build_log_step", static_cast<uint32_t>((100 * step) / total));
        system("ro");
        step++;
        g_server->send("build_log_step", static_cast<uint32_t>((100 * step) / total));
    }
    system(("rm -rf " + TEST_TMP_DIR).c_str());
    WARNING("Report generated at: %s", DEST_FILE.c_str())
}