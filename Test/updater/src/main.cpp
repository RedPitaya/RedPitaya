/**
 *
 * @brief Red Pitaya updater utility.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

#include "options.h"
#include "rp_updater.h"

/** Program name */
const char* g_argv0 = NULL;
Options g_option;
int g_returnValue = 0;

class Callback : public CUpdaterCallback {
   public:
    void clearLine() {
        std::cout << "\033[2K";
        std::cout << "\r";
    }

    void displayProgressBar(double progress, double speed) {
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

        if (speed >= 0) {
            if (speed < 1024) {
                std::cout << "(" << speed << " B/s)";
            } else if (speed < 1024 * 1024) {
                std::cout << "(" << (speed / 1024) << " KB/s)";
            } else {
                std::cout << "(" << (speed / (1024 * 1024)) << " MB/s)";
            }
        }

        std::cout.flush();
    }

    void reset() { m_needInit = true; }

    void downloadProgress(std::string fileName, uint64_t dnow, uint64_t dtotal, bool stop) override {
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

        if ((progress >= 1 || stop) && !m_printEnd) {
            m_printEnd = true;
            displayProgressBar(progress, speed);
            std::cout << std::endl;
        } else if (progress < 1) {
            displayProgressBar(progress, speed);
        }
    };

    void progressFiles(const std::string& prefix, const std::string& fileName, uint64_t dnow, uint64_t dtotal) {

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
        draw(prefix, progress, dnow, dtotal);
        if (dnow == dtotal) {
            std::cout << std::endl;
        }
    };

    void downloadDone(std::string fileName, bool success) override {
        g_returnValue = success ? 0 : -1;
        if (!g_option.verbose)
            return;
        if (success)
            std::cout << "File downloaded: " << fileName.c_str() << std::endl;
        else
            std::cout << "Error download file: " << fileName.c_str() << std::endl;
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

static void termSignalHandler(int) {
    rp_UpdaterStopDownloadFile();
}

static void installTermSignalHandler() {
    signal(SIGINT, termSignalHandler);
    signal(SIGTERM, termSignalHandler);
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

    if (option.calcMD5) {
        for (auto& file : option.filesForMD5) {
            std::string hash;
            if (rp_UpdaterGetMD5(file, &hash) == RP_UP_OK) {
                auto f = split(file, '/');
                if (f.size())
                    printf("%s %s\n", f.back().c_str(), hash.c_str());
            }
        }
    } else if (option.downloadURL) {
        installTermSignalHandler();
        auto callback = new Callback();
        callback->reset();
        rp_UpdaterSetCallback(callback);
        rp_UpdaterDownloadFileAsync(option.url);
        rp_UpdaterWaitDownloadFile();
        rp_UpdaterRemoveCallback();
        delete callback;
    } else if (option.listOflocal) {
        std::vector<std::string> files;
        rp_UpdaterGetDownloadedFilesList(files);
        for (auto& f : files) {
            bool isValid = false;
            rp_UpdaterIsValidDownloadedFile(f, &isValid);
            printf("%s\t%s\n", f.c_str(), isValid ? "[OK]" : "[BROKEN]");
        }

    } else if (option.listOfNB) {
        std::vector<std::string> files;
        rp_UpdaterGetNBAvailableFilesList(files);
        for (auto& f : files) {
            printf("%s\n", f.c_str());
        }

    } else if (option.downloadNB) {
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
                rp_UpdaterDownloadNBFileAsync(bn);
                rp_UpdaterWaitDownloadFile();
            } else {
                g_returnValue = -1;
            }
            rp_UpdaterRemoveCallback();
            delete callback;
        }
    } else if (option.install) {
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
            char buff[256];
            sprintf(buff, "systemctl stop redpitaya_e3_controller.service");
            auto ret = system(buff);
            if (ret != 0) {
                fprintf(stderr, "Error stop redpitaya_e3_controller.service");
                return -1;
            }

            sprintf(buff, "systemctl stop redpitaya_nginx.service");
            ret = system(buff);
            if (ret != 0) {
                fprintf(stderr, "Error stop redpitaya_nginx.service");
                return -1;
            }

            auto callback = new Callback();
            callback->reset();
            rp_UpdaterSetCallback(callback);
            g_returnValue = rp_UpdaterUpdateBoardEcosystem(file);
            rp_UpdaterRemoveCallback();
            delete callback;
            if (g_returnValue == 0)
                fprintf(stderr, "The board needs to be rebooted!!!!\n");
            else
                fprintf(stderr, "Fatal error while updating the ecosystem.\n");
        } else {
            g_returnValue = -1;
        }

    } else {
        usage(g_argv0);
    }

    rp_UpdaterRelease();
    return g_returnValue;
}
