/**
 *
 * @brief Red Pitaya updater utility.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <syslog.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "options.h"
#include "rp_updater.h"

#include "web/rp_websocket.h"

#define WEBPORT 9092

namespace fs = std::filesystem;

/** Program name */
const char* g_argv0 = NULL;
Options g_option;
int g_returnValue = 0;

std::atomic_bool g_stopWC = false;
std::atomic_bool g_needReboot = false;

rp_websocket::CWEBServer::Ptr g_server = nullptr;

std::thread* g_installThread = nullptr;

std::string formatFileSize(uintmax_t size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    if (size == 0)
        return "0 B";

    int unit_index = static_cast<int>(log2(size) / 10);
    unit_index = unit_index > 4 ? 4 : unit_index;

    double size_in_unit = size / pow(1024, unit_index);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f %s", size_in_unit, units[unit_index]);

    return buffer;
}

class Callback : public CUpdaterCallback {
   public:
    void clearLine() {
        std::cout << "\033[2K";
        std::cout << "\r";
    }

    void displayProgressBar(double progress, uint64_t downloaded, uint64_t total, double speed) {
        const int barWidth = 50;
        clearLine();

        std::cout << "[";
        int pos = static_cast<int>(barWidth * progress);
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] ";

        std::cout << std::fixed << std::setprecision(1) << (progress * 100.0) << "% ";

        std::cout << "(" << formatFileSize(downloaded) << " / " << formatFileSize(total) << ") ";

        if (!std::isinf(speed) && speed >= 0) {
            std::cout << "[" << formatFileSize(static_cast<uint64_t>(speed)) << "/s]";
        }

        std::cout.flush();
    }

    void reset() { m_needInit = true; }

    void downloadProgress(std::string fileName, uint64_t dnow, uint64_t dtotal, bool stop) override {
        static double prev_value = 0;
        if (!g_option.verbose)
            return;
        if (m_needInit) {
            m_startTime = std::chrono::steady_clock::now();
            m_needInit = false;
            m_printEnd = false;
        }
        double progress = static_cast<double>(dnow) / dtotal;
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime).count() / 1000.0;
        auto speed = dnow / elapsedTime;

        if (((int)prev_value == (int)(progress * 20.0)) && g_option.verbose_short)
            return;

        prev_value = progress * 20.0;

        if ((progress >= 1 || stop) && !m_printEnd) {
            m_printEnd = true;
            displayProgressBar(progress, dnow, dtotal, speed);
            std::cout << std::endl;
        } else if (progress < 1) {
            displayProgressBar(progress, dnow, dtotal, speed);
        }
    };

    void progressFiles(const std::string& prefix, const std::string& fileName, uint64_t dnow, uint64_t dtotal) {
        static double prev_value = 0;
        auto draw = [](const std::string& prefix, double progress, uint64_t dnow, uint64_t dtotal) {
            const int barWidth = 50;
            std::cout << "\033[2K";
            std::cout << "\r";
            std::cout << prefix.c_str();
            std::cout << "[";
            int pos = static_cast<int>(barWidth * progress);
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos)
                    std::cout << "=";
                else if (i == pos)
                    std::cout << ">";
                else
                    std::cout << " ";
            }
            std::cout << "] ";
            std::cout << std::fixed << std::setprecision(1) << (progress * 100.0) << "% ";
            std::cout << "(" << dnow << "/" << dtotal << ")";
            std::cout.flush();
        };

        if (!g_option.verbose)
            return;

        double progress = static_cast<double>(dnow) / dtotal;

        if (((int)prev_value == (int)(progress * 20.0)) && g_option.verbose_short)
            return;

        prev_value = progress * 20.0;

        draw(prefix, progress, dnow, dtotal);
        if (dnow == dtotal) {
            std::cout << std::endl;
        }
    };

    void downloadDone(std::string fileName, bool success) override {
        g_returnValue = success ? 0 : -1;
        if (!success)
            std::cerr << "Error download file: " << fileName.c_str() << std::endl;

        if (success && g_option.verbose)
            std::cout << "File downloaded: " << fileName.c_str() << std::endl;
    };

    void unzipProgress(uint64_t current, uint64_t total, const char* fileName) override {
        if (!g_option.verbose)
            return;
        progressFiles("Unzip   ", fileName, current, total);
    };

    void installProgress(uint64_t current, uint64_t total, const char* fileName) override {
        if (!g_option.verbose)
            return;
        progressFiles("Install ", fileName, current, total);
    };

    bool m_needInit = true;
    bool m_printEnd = false;
    std::chrono::_V2::steady_clock::time_point m_startTime;
};

class CallbackWC : public CUpdaterCallback {
   public:
    void downloadProgress(std::string fileName, uint64_t dnow, uint64_t dtotal, bool stop) override {
        if (g_server) {
            g_server->send("download_progress_now", static_cast<uint32_t>(dnow));
            g_server->send("download_progress_total", static_cast<uint32_t>(dtotal));
            g_server->send("download_progress_stop", stop);
        }
    };

    void downloadDone(std::string fileName, bool success) override {
        if (g_server) {
            g_server->send("download_done", success);
        }
    };

    void unzipProgress(uint64_t current, uint64_t total, const char* fileName) override {
        if (g_server) {
            g_server->send("unzip_progress_current", static_cast<uint32_t>(current));
            g_server->send("unzip_progress_total", static_cast<uint32_t>(total));
        }
    };

    void installProgress(uint64_t current, uint64_t total, const char* fileName) override {
        if (g_server) {
            g_server->send("install_progress_current", static_cast<uint32_t>(current));
            g_server->send("install_progress_total", static_cast<uint32_t>(total));
        }
    };

    void installDone(std::string fileName, bool success) override {
        if (g_server) {
            g_server->send("install_done", success);
        }
    };
};

static void termSignalHandler(int) {
    rp_UpdaterStopDownloadFile();
    g_stopWC = true;
}

static void installTermSignalHandler() {
    signal(SIGINT, termSignalHandler);
    signal(SIGTERM, termSignalHandler);
}

void startDaemon() {
    pid_t pid;
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x > 0; x--)
        close(x);
}

std::vector<std::string> readFile(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    if (!file)
        return lines;

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}

/** Acquire utility main */
int main(int argc, char* argv[]) {

    g_argv0 = argv[0];
    auto option = parse(argc, argv);
    g_option = option;

    if (option.error) {
        usage(g_argv0);
        return -1;
    }

    if (rp_UpdaterInit() != RP_UP_OK) {
        fprintf(stderr, "[Error] Can't init api\n");
        return -1;
    }

    if (option.mode == MD5) {
        for (auto& file : option.filesForMD5) {
            std::string hash;
            if (rp_UpdaterGetMD5(file, &hash) == RP_UP_OK) {
                auto f = split(file, '/');
                if (f.size())
                    printf("%s %s\n", f.back().c_str(), hash.c_str());
            }
        }
    } else if (option.mode == DOWNLOAD) {
        installTermSignalHandler();
        auto callback = new Callback();
        callback->reset();
        rp_UpdaterSetCallback(callback);
        rp_UpdaterDownloadFileAsync(option.url);
        rp_UpdaterWaitDownloadFile();
        rp_UpdaterRemoveCallback();
        delete callback;
    } else if (option.mode == LIST_LOCAL) {
        std::vector<std::string> files;
        rp_UpdaterGetDownloadedFilesList(files);
        for (auto& f : files) {
            bool isValid = false;
            rp_UpdaterIsValidDownloadedFile(f, &isValid);
            try {
                auto file_size = fs::file_size(std::string("/home/redpitaya/ecosystems/") + f);
                std::string formatted_size = formatFileSize(file_size);
                printf("%-50s %10s\t%s\n", f.c_str(), formatted_size.c_str(), isValid ? "[OK]" : "[BROKEN]");
            } catch (const fs::filesystem_error& e) {
                printf("%-50s %10s\t%s\n", f.c_str(), "N/A", isValid ? "[OK]" : "[BROKEN]");
            }
        }

    } else if (option.mode == LIST_NB) {
        std::vector<std::string> files;
        int ret = rp_UpdaterGetNBAvailableFilesList(files);
        if (ret == RP_UP_ERR) {
            fprintf(stderr, "Error getting file list from server\n");
            g_returnValue = -1;
        } else {
            for (auto& f : files) {
                printf("%s\n", f.c_str());
            }
        }

    } else if (option.mode == LIST_PROD) {
        std::vector<std::string> files;
        if (option.user.empty() && option.password.empty()) {
            auto cred = readFile(CRED_PATH);
            if (cred.size() == 2) {
                option.user = cred[0];
                option.password = cred[1];
            }
        }
        int ret = rp_UpdaterGetProductionAvailableFilesList(files, option.user, option.password);
        if (ret == RP_UP_ERR) {
            fprintf(stderr, "Error getting file list from server\n");
            g_returnValue = -1;
        } else {
            for (auto& f : files) {
                printf("%s\n", f.c_str());
            }
        }

    } else if (option.mode == DOWNLOAD_NB) {
        if (option.nbFileName != "") {
            auto callback = new Callback();
            callback->reset();
            rp_UpdaterSetCallback(callback);
            rp_UpdaterDownloadFileAsync(std::string("https://downloads.redpitaya.com/downloads/Unify/nightly_builds/") + option.nbFileName);
            rp_UpdaterWaitDownloadFile();
            delete callback;
        } else {
            auto callback = new Callback();
            callback->reset();
            rp_UpdaterSetCallback(callback);
            int bn = 0;
            try {
                bn = std::stoi(option.nbBuildNumber);
            } catch (...) {}
            if (bn != 0) {
                int ret = rp_UpdaterDownloadNBFileAsync(bn);
                if (ret == RP_UP_OK) {
                    rp_UpdaterWaitDownloadFile();
                } else if (ret == RP_UP_ERR || ret == RP_UP_EDF) {
                    fprintf(stderr, "Error getting file list from server\n");
                    g_returnValue = -1;
                } else if (ret == RP_UP_EFL) {
                    fprintf(stderr, "Error. The ecosystem was not found on the server.\n");
                    g_returnValue = -1;
                } else {
                    fprintf(stderr, "Error. Unknown error %d.\n", ret);
                    g_returnValue = -1;
                }
            } else {
                g_returnValue = -1;
            }
            rp_UpdaterRemoveCallback();
            delete callback;
        }
    } else if (option.mode == DOWNLOAD_PROD) {
        if (option.user.empty() && option.password.empty()) {
            auto cred = readFile(CRED_PATH);
            if (cred.size() == 2) {
                option.user = cred[0];
                option.password = cred[1];
            }
        }
        if (option.nbFileName != "") {
            auto callback = new Callback();
            callback->reset();
            rp_UpdaterSetCallback(callback);
            rp_UpdaterDownloadFileAsync(std::string("https://downloads.redpitaya.com/production/ecosystem/") + option.nbFileName, option.user, option.password);
            rp_UpdaterWaitDownloadFile();
            delete callback;
        } else {
            auto callback = new Callback();
            callback->reset();
            rp_UpdaterSetCallback(callback);
            int bn = 0;
            try {
                bn = std::stoi(option.nbBuildNumber);
            } catch (...) {}
            if (bn != 0) {
                int ret = rp_UpdaterDownloadProductionFileAsync(bn, option.user, option.password);
                if (ret == RP_UP_OK) {
                    rp_UpdaterWaitDownloadFile();
                } else if (ret == RP_UP_ERR || ret == RP_UP_EDF) {
                    fprintf(stderr, "Error getting file list from server\n");
                    g_returnValue = -1;
                } else if (ret == RP_UP_EFL) {
                    fprintf(stderr, "Error. The ecosystem was not found on the server.\n");
                    g_returnValue = -1;
                } else {
                    fprintf(stderr, "Error. Unknown error %d.\n", ret);
                    g_returnValue = -1;
                }
            } else {
                g_returnValue = -1;
            }
            rp_UpdaterRemoveCallback();
            delete callback;
        }
    } else if (option.mode == INSTALL) {
        std::string file = option.installFileName;
        if (file == "") {
            int bn = 0;
            try {
                bn = std::stoi(option.installNumber);
            } catch (...) {}
            if (bn != 0) {
                uint32_t count = 0;
                rp_UpdaterGetDownloadedCount(&count);
                for (uint32_t i = 0; i < count; i++) {
                    std::string name = {};
                    std::string commit = {};
                    uint32_t number = 0;
                    if (rp_UpdaterGetDownloadedFile(i, &name, &number, &commit) == RP_UP_OK) {
                        if ((int)number == bn) {
                            file = name;
                            break;
                        }
                    }
                }
            } else {
                g_returnValue = -1;
            }
        }
        if (file != "") {

            std::ifstream file_version("/root/.version");
            if (file_version.is_open()) {
                std::string line;
                std::getline(file_version, line);
                file_version.close();
                if (file.find(line) == std::string::npos) {
                    fprintf(stderr, "Warning. The current OS version does not match the one being installed.\n");
                }
            } else {
                fprintf(stderr, "Warning. Can't open file to check OS version\n");
            }

            char buff[256];
            sprintf(buff, "systemctl stop redpitaya_e3_controller.service");
            auto ret = system(buff);
            if (ret != 0) {
                fprintf(stderr, "Error stop redpitaya_e3_controller.service");
                g_returnValue = -1;
            }

            sprintf(buff, "systemctl stop redpitaya_nginx.service");
            ret = system(buff);
            if (ret != 0) {
                fprintf(stderr, "Error stop redpitaya_nginx.service");
                g_returnValue = -1;
            }
            if (g_returnValue == 0) {
                auto callback = new Callback();
                callback->reset();
                rp_UpdaterSetCallback(callback);
                g_returnValue = rp_UpdaterUpdateBoardEcosystem(file, false);
                rp_UpdaterRemoveCallback();
                delete callback;
            }
            if (g_returnValue == 0)
                fprintf(stderr, "The board needs to be rebooted!!!!\n");
            else
                fprintf(stderr, "Fatal error while updating the ecosystem.\n");
        } else {
            fprintf(stderr, "Error: No ecosystem found to install.\n");
            g_returnValue = -1;
        }

    } else if (option.webcontrol) {

        openlog("updater", LOG_PID, LOG_USER);

        auto callback = new CallbackWC();

        rp_UpdaterSetCallback(callback);
        g_server = std::make_shared<rp_websocket::CWEBServer>();
        g_server->startServer(WEBPORT);
        g_server->receiveInt.connect([](auto key, auto command) {
            if (key == "stop") {
                g_stopWC = true;
            }
            if (key == "reboot") {
                g_stopWC = true;
                g_needReboot = true;
            }
        });

        g_server->receiveStr.connect([](auto key, auto value) {
            if (key == "download") {
                auto ret = rp_UpdaterDownloadFileAsync(std::string(value));
                g_server->send("download", ret);
            }

            if (key == "install") {

                if (g_installThread && g_installThread->joinable()) {
                    g_installThread->join();
                    delete g_installThread;
                    g_installThread = nullptr;
                }

                if (g_installThread == nullptr) {
                    auto fileName = std::string(value);
                    g_installThread = new std::thread([fileName]() {
                        auto ret = rp_UpdaterUpdateBoardEcosystem(std::string(fileName), true);
                        g_server->send("install", ret);
                    });
                }
            }
        });

        while (!g_stopWC) {
            usleep(10000);
        }
        if (g_installThread && g_installThread->joinable()) {
            g_installThread->join();
        }
        rp_UpdaterRemoveCallback();
        if (g_needReboot) {
            syslog(LOG_NOTICE, "Start reboot");
            system("systemd-run --on-active=5s --unit=shutdown5s systemctl reboot");
        }
        closelog();
    } else {
        usage(g_argv0);
    }

    rp_UpdaterRelease();
    return g_returnValue;
}
