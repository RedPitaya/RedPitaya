/**
 * $Id: $
 *
 * @brief Red Pitaya library updater api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __UPDATER_API_H
#define __UPDATER_API_H

#include <string>
#include <vector>
#include "rp_updater_common.h"

class CUpdaterCallback {
   public:
    virtual ~CUpdaterCallback() {}

    virtual void downloadProgress(std::string fileName, uint64_t dnow, uint64_t dtotal, bool stop) {}

    virtual void downloadDone(std::string fileName, bool success) {}

    virtual void unzipProgress(uint64_t current, uint64_t total, const char* fileName) {};

    virtual void installProgress(uint64_t current, uint64_t total, const char* fileName) {};

    virtual void installDone(std::string fileName, bool success) {}
};

int rp_UpdaterInit();
int rp_UpdaterRelease();
int rp_UpdaterGetFreeSapce(std::string path, uint64_t* size);

int rp_UpdaterGetDownloadedFiles();
int rp_UpdaterGetDownloadedCount(uint32_t* count);
int rp_UpdaterGetDownloadedFile(uint32_t _index, std::string* name, uint32_t* build_number, std::string* commit);
int rp_UpdaterGetDownloadedFilesList(std::vector<std::string>& files);
int rp_UpdaterIsValidDownloadedFile(std::string fileName, bool* state);

int rp_UpdaterDownloadFile(std::string url, const std::string& username = "", const std::string& password = "");
int rp_UpdaterDownloadFileAsync(std::string url, const std::string& username = "", const std::string& password = "");

int rp_UpdaterWaitDownloadFile();
int rp_UpdaterStopDownloadFile();

int rp_UpdaterSetCallback(CUpdaterCallback* callbacks);
int rp_UpdaterRemoveCallback();

// Nightly build functions
int rp_UpdaterGetNBAvailableFilesList(std::vector<std::string>& files);
int rp_UpdaterDownloadNBFile(uint32_t number);
int rp_UpdaterDownloadNBFileAsync(uint32_t number);

// Production build functions
int rp_UpdaterGetProductionAvailableFilesList(std::vector<std::string>& files, const std::string& username, const std::string& password);
int rp_UpdaterDownloadProductionFile(uint32_t number, const std::string& username, const std::string& password);
int rp_UpdaterDownloadProductionFileAsync(uint32_t number, const std::string& username, const std::string& password);

// Release build functions
int rp_UpdaterGetReleaseAvailableFilesList(std::vector<std::string>& files);

int rp_UpdaterUpdateBoardEcosystem(std::string fileName, bool stopServices);

#endif  // __UPDATER_API_H
