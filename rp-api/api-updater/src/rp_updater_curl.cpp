/**
 * $Id: $
 *
 * @brief Red Pitaya library updater api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include "rp_updater_curl.h"
#include <filesystem>
#include <iostream>
#include <regex>
#include "rp_log.h"

namespace fs = std::filesystem;

size_t curl_write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

size_t curl_write_html_data(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

int CUCurl::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if (clientp != NULL) {
        auto ptr = static_cast<CUCurl*>(clientp);
        if (dltotal > 204) {
            if (ptr->m_delegate != nullptr) {
                ptr->m_delegate(dlnow, dltotal, ptr->stop_download);
            }
        }
        if (ptr->stop_download) {
            return 1;
        }
    }
    return 0;
}

std::vector<std::string> extractLinks(const std::string& html) {
    std::vector<std::string> links;
    std::regex link_regex(R"xyz(<a\s+(?:[^>]*?\s+)?href="([^"]*\.(zip|tar\.gz)[^"]*)")xyz", std::regex_constants::icase);
    auto words_begin = std::sregex_iterator(html.begin(), html.end(), link_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string link = match.str(1);
        links.push_back(link);
    }

    return links;
}

CUCurl::CUCurl() {
    m_delegate = nullptr;
    m_downloadTh = nullptr;
    m_done = nullptr;
    stop_download = false;
}

CUCurl::~CUCurl() {
    stopDownloadFile();
    wait();
    m_delegate = nullptr;
    m_done = nullptr;
}

auto CUCurl::stopDownloadFile() -> bool {
    stop_download = true;
    return true;
}

auto CUCurl::wait() -> void {
    if (m_downloadTh) {
        if (m_downloadTh->joinable()) {
            m_downloadTh->join();
        }
        delete m_downloadTh;
        m_downloadTh = nullptr;
    }
}

auto CUCurl::downloadFile(const std::string& url, const std::string& output_file, const std::string& username, const std::string& password) -> int {
    CURL* curl = NULL;
    FILE* fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(output_file.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);  // 10 sec
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CUCurl::progressCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        if (!username.empty() || !password.empty()) {
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
            curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        }
        res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);
        fclose(fp);

        if (res == CURLE_OK && http_code == 200) {
            return RP_UP_OK;
        }
    }
    return RP_UP_EDF;
}

auto CUCurl::downloadFileAsync(const std::string& url, const std::string& output_file, const std::string& username, const std::string& password) -> int {
    stopDownloadFile();
    wait();
    stop_download = false;
    m_downloadTh = new std::thread(
        [username, password](CUCurl* holder, const std::string url, const std::string output_file) {
            CURL* curl = NULL;
            FILE* fp;
            CURLcode res;

            curl = curl_easy_init();
            if (curl) {
                fp = fopen(output_file.c_str(), "wb");
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
                curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);  // 10 sec
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                curl_easy_setopt(curl, CURLOPT_XFERINFODATA, holder);
                curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CUCurl::progressCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                if (!username.empty() || !password.empty()) {
                    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
                    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
                    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
                }
                res = curl_easy_perform(curl);
                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                curl_easy_cleanup(curl);
                fclose(fp);

                if (res == CURLE_OK && http_code == 200) {
                    if (holder->m_done) {
                        holder->m_done(true);
                    }
                    return;
                }
                fs::remove(output_file);
            }
            if (holder->m_done) {
                holder->m_done(false);
            }
        },
        this, url, output_file);
    return RP_UP_OK;
}

auto CUCurl::setProgressCallback(func_progress_t func) -> void {
    m_delegate = func;
}

auto CUCurl::setDoneCallback(func_done_t func) -> void {
    m_done = func;
}

auto CUCurl::getFilenameFromUrl(const std::string& url) -> std::string {
    std::regex regex(R"(([^/?#]+)(?:[?#].*)?$)");
    std::smatch match;

    if (std::regex_search(url, match, regex)) {
        return match.str(1);
    }

    return "";
}

auto CUCurl::getListNB(bool* succes) -> std::vector<std::string> {
    CURL* curl;
    CURLcode res;
    std::string html;
    *succes = false;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, NB_LINK);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_html_data);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return {};
        }
        *succes = true;
        return extractLinks(html);
    }
    return {};
}

auto CUCurl::getListRelease(bool* succes) -> std::vector<std::string> {
    CURL* curl;
    CURLcode res;
    std::string html;
    *succes = false;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, RELEASE_LINK);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_html_data);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return {};
        }
        *succes = true;
        return extractLinks(html);
    }
    return {};
}

auto CUCurl::getListProduction(bool* success, const std::string& username, const std::string& password) -> std::vector<std::string> {
    CURL* curl;
    CURLcode res;
    std::string html;
    *success = false;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, PRODUCTION_LINK);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_html_data);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
        if (!username.empty() || !password.empty()) {
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
            curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        }
        res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);
        if (res == CURLE_OK) {
            if (http_code == 200) {
                *success = true;
                return extractLinks(html);
            } else if (http_code == 401) {
                std::cerr << "Authorization failed (HTTP 401)" << std::endl;
            } else {
                std::cerr << "HTTP error: " << http_code << std::endl;
            }
        } else {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        }
        return {};
    }
    return {};
}