/**
 * $Id: $
 *
 * @brief Red Pitaya library updater api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __UPDATER_CURL_API_H
#define __UPDATER_CURL_API_H

#include <curl/curl.h>
#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include "rp_updater_common.h"

#define NB_LINK "https://downloads.redpitaya.com/downloads/Unify/nightly_builds/"
#define RELEASE_LINK "https://downloads.redpitaya.com/downloads/Unify/ecosystems/"
#define PRODUCTION_LINK "https://downloads.redpitaya.com/production/ecosystem/"

class CUCurl {
    typedef std::function<void(uint64_t now, uint64_t total, bool stop)> func_progress_t;
    typedef std::function<void(bool success)> func_done_t;

   public:
    CUCurl();
    ~CUCurl();

    CUCurl(CUCurl&) = delete;
    CUCurl(CUCurl&&) = delete;

    auto downloadFile(const std::string& url, const std::string& output_file, const std::string& username = "", const std::string& password = "") -> int;
    auto downloadFileAsync(const std::string& url, const std::string& output_file, const std::string& username = "", const std::string& password = "") -> int;
    auto stopDownloadFile() -> bool;
    auto wait() -> void;

    auto setProgressCallback(func_progress_t func) -> void;
    auto setDoneCallback(func_done_t func) -> void;

    auto getListNB(bool* succes) -> std::vector<std::string>;
    auto getListRelease(bool* succes) -> std::vector<std::string>;
    auto getListProduction(bool* succes, const std::string& username = "", const std::string& password = "") -> std::vector<std::string>;

    static auto getFilenameFromUrl(const std::string& url) -> std::string;

   private:
    func_progress_t m_delegate;
    func_done_t m_done;
    std::thread* m_downloadTh;
    std::atomic<bool> stop_download;
    static auto progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) -> int;
};

#endif  // __UPDATER_CURL_API_H
