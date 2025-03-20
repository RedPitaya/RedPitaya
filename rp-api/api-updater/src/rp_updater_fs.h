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

#ifndef __UPDATER_FS_API_H
#define __UPDATER_FS_API_H

#include <functional>
#include <vector>
#include "rp_updater_common.h"

auto createDirTree(const std::string& full_path) -> bool;
auto copyFile(const std::string& _src, const std::string& _dst) -> void;
auto removeDirectory(const std::string& path) -> bool;
auto deleteFiles(std::vector<std::string>& files_for_delete) -> void;
auto listdir(const std::string& root, const std::string& d_name, int level, std::vector<std::string>& dirs,
             std::vector<std::pair<std::string, std::string>>& files) -> void;
auto listdirForDelete(const std::string& root, const std::string& d_name, int level, std::vector<std::pair<std::string, std::string>>& files,
                      std::vector<std::string>& files_for_delete) -> void;
auto readFileFromZip(const std::string& zip_path, const std::string& file_name) -> std::string;
auto readFileFromZipBytes(const std::string& zip_path, const std::string& file_name) -> std::vector<uint8_t>;
auto unzip(const std::string& zip_path, const std::string& output_dir,
           std::function<void(uint64_t current, uint64_t total, const char* fileName)> progress) -> int;

#endif  // __UPDATER_FS_API_H
