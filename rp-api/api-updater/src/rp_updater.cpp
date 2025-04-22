/**
 * $Id: $
 *
 * @brief Red Pitaya library updater api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include "rp_updater.h"

#include <signal.h>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include "rp_log.h"
#include "rp_updater_curl.h"
#include "rp_updater_fs.h"

#define NAME_LEN 20

CUCurl* g_curl = nullptr;
CUpdaterCallback* g_callbacks = nullptr;
std::mutex g_curlMutex;
std::mutex g_callbackMutex;

std::vector<std::string> g_dirs;
std::vector<std::pair<std::string, std::string>> g_files;
std::vector<std::string> g_files_for_delete;

namespace fs = std::filesystem;

struct info_t {
    std::string name;
    std::string commit;
    uint32_t buildNumber;
};

void signalHandlerStrong(int signum) {}

void signalHandlerDefault(int signum) {
    exit(0);
}

auto getUpdaterAvailableSpace(std::string dst, uint64_t* availableSize) -> int {
    std::error_code ec;
    const std::filesystem::space_info si = std::filesystem::space(dst, ec);
    if (!ec) {
        *availableSize = si.available;
        return 0;
    } else {
        std::filesystem::path path = dst;
        if (path.has_parent_path()) {
            return getUpdaterAvailableSpace(path.parent_path().generic_string(), availableSize);
        } else {
            WARNING("Error: %s\n", ec.message().c_str());
            *availableSize = 0;
        }
    }
    return RP_UP_EGFS;
}

auto getUpdaterFreeSpaceDisk(std::string _filePath, uint64_t* availableSize) -> int {
    return getUpdaterAvailableSpace(_filePath, availableSize);
}

std::vector<std::string> split(const std::string& str, const std::string& delimiters) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find_first_of(delimiters);

    while (end != std::string::npos) {
        if (end != start) {
            tokens.push_back(str.substr(start, end - start));
        }
        start = end + 1;
        end = str.find_first_of(delimiters, start);
    }

    if (start < str.size()) {
        tokens.push_back(str.substr(start));
    }

    return tokens;
}

void copyEcosystem() {
    uint64_t total = g_dirs.size() + g_files.size();
    uint64_t current = 1;
    for (auto& item : g_dirs) {
        createDirTree(item.c_str());
        std::lock_guard lock(g_callbackMutex);
        if (g_callbacks) {
            g_callbacks->installProgress(current++, total, item.c_str());
        }
    }
    for (size_t i = 0; i < g_files.size(); i++) {
        copyFile(g_files[i].first.c_str(), g_files[i].second.c_str());
        std::lock_guard lock(g_callbackMutex);
        if (g_callbacks) {
            g_callbacks->installProgress(current++, total, g_files[i].second.c_str());
        }
    }
}

std::vector<std::pair<std::string, info_t>> g_downloadedFiles;

int rp_UpdaterInit() {
    fs::create_directories(ECOSYSTEM_DOWNLOAD_PATH);
    fs::permissions(ECOSYSTEM_DOWNLOAD_PATH, std::filesystem::perms::owner_all | std::filesystem::perms::group_all, std::filesystem::perm_options::add);
    return rp_UpdaterGetDownloadedFiles();
}

int rp_UpdaterRelease() {
    rp_UpdaterStopDownloadFile();
    std::lock_guard lock(g_curlMutex);
    delete g_curl;
    g_curl = nullptr;
    return RP_UP_OK;
}

int rp_UpdaterGetFreeSapce(std::string path, uint64_t* size) {
    return getUpdaterFreeSpaceDisk(path, size);
}

int rp_UpdaterGetDownloadedFiles() {
    g_downloadedFiles.clear();
    for (const auto& entry : fs::directory_iterator(ECOSYSTEM_DOWNLOAD_PATH)) {
        TRACE_SHORT("Load file %s", entry.path().filename().c_str())
        std::vector<std::string> seglist = split(entry.path().filename().string(), ".-");
        if (seglist.size() == 6) {
            g_downloadedFiles.push_back(
                {entry.path().filename(), {.name = entry.path().filename().string(), .commit = seglist[4], .buildNumber = (uint32_t)std::stoi(seglist[3])}});
        }
    }
    return RP_UP_OK;
}

int rp_UpdaterGetDownloadedCount(uint32_t* count) {
    *count = g_downloadedFiles.size();
    return RP_UP_OK;
}

int rp_UpdaterGetDownloadedFile(uint32_t _index, std::string* name, uint32_t* build_number, std::string* commit) {
    *name = "";
    *build_number = 0;
    *commit = "";
    if (_index >= g_downloadedFiles.size()) {
        return RP_UP_EWI;
    }
    *name = g_downloadedFiles.at(_index).second.name;
    *build_number = g_downloadedFiles.at(_index).second.buildNumber;
    *commit = g_downloadedFiles.at(_index).second.commit;
    return RP_UP_OK;
}

int rp_UpdaterGetDownloadedFilesList(std::vector<std::string>& files) {
    files.clear();
    for (auto& itm : g_downloadedFiles) {
        files.push_back(itm.second.name);
    }
    std::sort(files.begin(), files.end());
    return RP_UP_OK;
}

int rp_UpdaterDownloadFile(std::string url) {
    CUCurl curl;
    auto fileName = CUCurl::getFilenameFromUrl(url);
    curl.setDoneCallback([fileName](bool success) {
        std::lock_guard lock(g_callbackMutex);
        if (g_callbacks) {
            g_callbacks->downloadDone(fileName, success);
        }
    });
    curl.setProgressCallback([fileName](uint64_t now, uint64_t total, bool stop) {
        std::lock_guard lock(g_callbackMutex);
        if (g_callbacks) {
            g_callbacks->downloadProgress(fileName, now, total, stop);
        }
    });
    return curl.downloadFile(url, std::string(ECOSYSTEM_DOWNLOAD_PATH) + "/" + fileName);
}

int rp_UpdaterDownloadFileAsync(std::string url) {
    std::lock_guard lock(g_curlMutex);
    delete g_curl;
    g_curl = new CUCurl();
    auto fileName = CUCurl::getFilenameFromUrl(url);
    g_curl->setDoneCallback([fileName](bool success) {
        std::lock_guard lock(g_callbackMutex);
        if (g_callbacks) {
            g_callbacks->downloadDone(fileName, success);
        }
    });
    g_curl->setProgressCallback([fileName](uint64_t now, uint64_t total, bool stop) {
        std::lock_guard lock(g_callbackMutex);
        if (g_callbacks) {
            g_callbacks->downloadProgress(fileName, now, total, stop);
        }
    });
    return g_curl->downloadFileAsync(url, std::string(ECOSYSTEM_DOWNLOAD_PATH) + "/" + fileName);
}
int rp_UpdaterWaitDownloadFile() {
    std::lock_guard lock(g_curlMutex);
    if (g_curl) {
        g_curl->wait();
    }
    return RP_UP_OK;
}

int rp_UpdaterStopDownloadFile() {
    if (g_curl) {
        g_curl->stopDownloadFile();
    }
    return RP_UP_OK;
}

int rp_UpdaterSetCallback(CUpdaterCallback* callbacks) {
    std::lock_guard lock(g_callbackMutex);
    g_callbacks = callbacks;
    return RP_UP_OK;
}

int rp_UpdaterRemoveCallback() {
    std::lock_guard lock(g_callbackMutex);
    g_callbacks = nullptr;
    return RP_UP_OK;
}

int rp_UpdaterDownloadNBFile(uint32_t number) {
    CUCurl curl;
    std::string url = "";
    bool succes = false;
    auto links = curl.getListNB(&succes);
    for (auto& link : links) {
        std::vector<std::string> seglist = split(link, ".-");
        if (seglist.size() > 3) {
            if ((uint32_t)std::stoi(seglist[3]) == number) {
                url = link;
                break;
            }
        }
    }

    if (succes == false) {
        return RP_UP_ERR;
    }

    if (url == "") {
        return RP_UP_EFL;
    }
    return rp_UpdaterDownloadFile(std::string(NB_LINK) + url);
}

int rp_UpdaterDownloadNBFileAsync(uint32_t number) {
    CUCurl curl;
    std::string url = "";
    bool succes = false;
    auto links = curl.getListNB(&succes);
    for (auto& link : links) {
        std::vector<std::string> seglist = split(link, ".-");
        if (seglist.size() > 3) {
            if ((uint32_t)std::stoi(seglist[3]) == number) {
                url = link;
                break;
            }
        }
    }

    if (succes == false) {
        return RP_UP_ERR;
    }

    if (url == "") {
        return RP_UP_EFL;
    }
    return rp_UpdaterDownloadFileAsync(std::string(NB_LINK) + url);
}

int rp_UpdaterGetNBAvailableFilesList(std::vector<std::string>& files) {
    files.clear();
    CUCurl curl;
    std::string url = "";
    bool succes = false;
    auto links = curl.getListNB(&succes);
    for (auto& link : links) {
        files.push_back(link);
    }
    if (succes == false) {
        return RP_UP_ERR;
    }
    return RP_UP_OK;
}

int rp_UpdaterIsValidDownloadedFile(std::string fileName, bool* state) {
    *state = false;
    auto md5 = readFileFromZip(std::string(ECOSYSTEM_DOWNLOAD_PATH) + "/" + fileName, "md5.txt");
    if (md5 == "") {
        return RP_UP_OK;
    }

    auto bootbin = readFileFromZipBytes(std::string(ECOSYSTEM_DOWNLOAD_PATH) + "/" + fileName, "boot.bin");
    if (bootbin.size() == 0) {
        return RP_UP_OK;
    }

    auto ubootscr = readFileFromZipBytes(std::string(ECOSYSTEM_DOWNLOAD_PATH) + "/" + fileName, "u-boot.scr");
    if (ubootscr.size() == 0) {
        return RP_UP_OK;
    }
    auto uImage = readFileFromZipBytes(std::string(ECOSYSTEM_DOWNLOAD_PATH) + "/" + fileName, "uImage");
    if (uImage.size() == 0) {
        return RP_UP_OK;
    }

    md5.erase(std::remove(md5.begin(), md5.end(), ' '), md5.end());

    std::string bootbin_hash;
    rp_UpdaterGetMD5(bootbin, &bootbin_hash);
    bootbin_hash += "boot.bin";

    std::string ubootscr_hash;
    rp_UpdaterGetMD5(ubootscr, &ubootscr_hash);
    ubootscr_hash += "u-boot.scr";

    std::string uImage_hash;
    rp_UpdaterGetMD5(uImage, &uImage_hash);
    uImage_hash += "uImage";

    *state = true;
    if (md5.find(bootbin_hash) == std::string::npos) {
        *state = false;
    }

    *state = true;
    if (md5.find(ubootscr_hash) == std::string::npos) {
        *state = false;
    }

    *state = true;
    if (md5.find(uImage_hash) == std::string::npos) {
        *state = false;
    }

    return RP_UP_OK;
}

bool replace_self(std::string& new_executable_path) {
    // Get current executable path
    std::string self_path;
    try {
        self_path = fs::canonical("/proc/self/exe");
    } catch (...) {
        fprintf(stderr, "Error getting executable path\n");
        return false;
    }

    // Create a temporary file in the same directory
    std::string temp_path = self_path + ".tmp";

    // Copy new executable to temporary file
    try {
        fs::copy_file(new_executable_path, temp_path, fs::copy_options::overwrite_existing);
    } catch (...) {
        fprintf(stderr, "Error copying new executable\n");
        return false;
    }

    // Set permissions on the new file
    if (chmod(temp_path.c_str(), 0755) != 0) {
        fprintf(stderr, "Error setting permissions\n");
        return false;
    }

    // Replace the old executable (atomic operation)
    if (rename(temp_path.c_str(), self_path.c_str()) != 0) {
        fprintf(stderr, "Error replacing executable\n");
        return false;
    }

    return true;
}

int rp_UpdaterUpdateBoardEcosystem(std::string fileName, bool stopServices) {
    bool valid = false;
    rp_UpdaterIsValidDownloadedFile(fileName, &valid);
    if (!valid) {
        return RP_UP_ECM;
    }
    auto out_dir = std::string("/tmp/") + fileName;
    removeDirectory(out_dir);
    auto ret = unzip(std::string(ECOSYSTEM_DOWNLOAD_PATH) + "/" + fileName, out_dir, [](uint64_t current, uint64_t total, const char* fileName) {
        std::lock_guard lock(g_callbackMutex);
        if (g_callbacks) {
            g_callbacks->unzipProgress(current, total, fileName);
        }
    });

    if (ret != RP_UP_OK) {
        return ret;
    }

    signal(SIGCHLD, signalHandlerStrong);
    signal(SIGHUP, signalHandlerStrong);
    signal(SIGABRT, signalHandlerStrong);
    signal(SIGFPE, signalHandlerStrong);
    signal(SIGILL, signalHandlerStrong);
    signal(SIGINT, signalHandlerStrong);
    signal(SIGSEGV, signalHandlerStrong);
    signal(SIGTERM, signalHandlerStrong);
    signal(SIGUSR1, signalHandlerStrong);
    signal(SIGUSR2, signalHandlerStrong);

    if (stopServices) {
        system("systemctl stop redpitaya_e3_controller.service");
        system("systemctl stop redpitaya_nginx.service");
    }

    char buff[256];
    sprintf(buff, "mount -o rw,remount  %s", ECOSYSTEM_INSTALL_PATH);
    ret = system(buff);
    if (ret != 0) {
        signal(SIGCHLD, signalHandlerDefault);
        signal(SIGHUP, signalHandlerDefault);
        signal(SIGABRT, signalHandlerDefault);
        signal(SIGFPE, signalHandlerDefault);
        signal(SIGILL, signalHandlerDefault);
        signal(SIGINT, signalHandlerDefault);
        signal(SIGSEGV, signalHandlerDefault);
        signal(SIGTERM, signalHandlerDefault);
        signal(SIGUSR1, signalHandlerDefault);
        signal(SIGUSR2, signalHandlerDefault);
        fprintf(stderr, "Error re-mount %d\n", ret);
        return RP_UP_ERM;
    }

    createDirTree(ECOSYSTEM_INSTALL_PATH);
    g_dirs.clear();
    g_files.clear();
    listdir(out_dir, "", 0, g_dirs, g_files);
    listdirForDelete(ECOSYSTEM_INSTALL_PATH, "", 0, g_files, g_files_for_delete);
    copyEcosystem();

    for (auto& path : g_files) {
        std::size_t found = path.first.find("/bin/updater");
        if (found != std::string::npos) {
            replace_self(path.first);
            break;
        }
    }

    deleteFiles(g_files_for_delete);
    deleteEmptyFolders("/opt/redpitaya");

    signal(SIGCHLD, signalHandlerDefault);
    signal(SIGHUP, signalHandlerDefault);
    signal(SIGABRT, signalHandlerDefault);
    signal(SIGFPE, signalHandlerDefault);
    signal(SIGILL, signalHandlerDefault);
    signal(SIGINT, signalHandlerDefault);
    signal(SIGSEGV, signalHandlerDefault);
    signal(SIGTERM, signalHandlerDefault);
    signal(SIGUSR1, signalHandlerDefault);
    signal(SIGUSR2, signalHandlerDefault);

    if (g_callbacks) {
        g_callbacks->installDone(fileName, true);
    }

    return ret;
}
