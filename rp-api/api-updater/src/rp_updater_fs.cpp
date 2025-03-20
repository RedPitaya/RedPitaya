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

#include "rp_updater_fs.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zip.h>
#include <cstring>
#include <fstream>
#include <iostream>

auto createDir(const std::string& dir) -> bool {
    mkdir(dir.c_str(), 0755);
    return true;
}

auto createDirTree(const std::string& full_path) -> bool {
    char ch = '/';

    size_t pos = 0;
    bool ret_val = true;
    while (ret_val == true && pos != std::string::npos) {
        pos = full_path.find(ch, pos + 1);
        ret_val = createDir(full_path.substr(0, pos));
    }
    return ret_val;
}

auto copyFile(const std::string& _src, const std::string& _dst) -> void {
    using namespace std;
    ifstream source(_src, ios::binary);
    ofstream dest(_dst, ios::binary);

    dest << source.rdbuf();

    source.close();
    dest.close();
}

auto deleteFiles(std::vector<std::string>& files_for_delete) -> void {
    for (auto item : files_for_delete) {
        std::remove(item.c_str());
    }
}

auto removeDirectory(const std::string& path) -> bool {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        std::string full_path = path + "/" + entry->d_name;

        struct stat stat_buf;
        if (stat(full_path.c_str(), &stat_buf) == -1) {
            closedir(dir);
            return false;
        }

        if (S_ISDIR(stat_buf.st_mode)) {
            if (!removeDirectory(full_path)) {
                closedir(dir);
                return false;
            }
        } else {
            if (unlink(full_path.c_str()) == -1) {
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);

    if (rmdir(path.c_str()) == -1) {
        return false;
    }

    return true;
}

auto listdir(const std::string& root, const std::string& d_name, int level, std::vector<std::string>& dirs,
             std::vector<std::pair<std::string, std::string>>& files) -> void {
    DIR* dir;
    struct dirent* entry;

    char name[1024];
    int len = snprintf(name, sizeof(name) - 1, "%s%s", root.c_str(), d_name.c_str());
    name[len] = 0;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char path[1024];
            int len = snprintf(path, sizeof(path) - 1, "%s/%s", d_name.c_str(), entry->d_name);
            path[len] = 0;
            std::string dir(ECOSYSTEM_INSTALL_PATH);
            dir = dir + path;
            dirs.push_back(dir);
            listdir(root, path, level + 1, dirs, files);
        } else {
            std::string from(name);
            from = from + "/" + entry->d_name;
            std::string to(ECOSYSTEM_INSTALL_PATH);
            to = to + d_name + "/" + entry->d_name;
            files.push_back({from, to});
        }
    } while ((entry = readdir(dir)));

    closedir(dir);
}

auto listdirForDelete(const std::string& root, const std::string& d_name, int level, std::vector<std::pair<std::string, std::string>>& files,
                      std::vector<std::string>& files_for_delete) -> void {
    DIR* dir;
    struct dirent* entry;

    char name[1024];
    int len = snprintf(name, sizeof(name) - 1, "%s%s", root.c_str(), d_name.c_str());
    name[len] = 0;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char path[1024];
            int len = snprintf(path, sizeof(path) - 1, "%s/%s", d_name.c_str(), entry->d_name);
            path[len] = 0;
            listdirForDelete(root, path, level + 1, files, files_for_delete);
        } else {
            std::string from(name);
            from = from + "/" + entry->d_name;
            bool add = true;
            for (size_t i = 0; i < files.size(); i++) {
                if (files[i].second == from) {
                    add = false;
                    break;
                }
            }
            if (add)
                files_for_delete.push_back(from);
        }
    } while ((entry = readdir(dir)));

    closedir(dir);
}

auto readFileFromZip(const std::string& zip_path, const std::string& file_name) -> std::string {
    int error = 0;
    zip_t* archive = zip_open(zip_path.c_str(), 0, &error);

    if (!archive) {
        return "";
    }

    zip_int64_t file_index = zip_name_locate(archive, file_name.c_str(), 0);
    if (file_index < 0) {
        zip_close(archive);
        return "";
    }

    zip_file_t* file = zip_fopen_index(archive, file_index, 0);
    if (!file) {
        zip_close(archive);
        return "";
    }

    std::string content;
    char buffer[1024];
    zip_int64_t bytes_read;
    while ((bytes_read = zip_fread(file, buffer, sizeof(buffer)))) {
        if (bytes_read < 0) {
            zip_fclose(file);
            zip_close(archive);
            return "";
        }
        content.append(buffer, bytes_read);
    }

    zip_fclose(file);
    zip_close(archive);

    return content;
}

auto readFileFromZipBytes(const std::string& zip_path, const std::string& file_name) -> std::vector<uint8_t> {
    int error = 0;
    zip_t* archive = zip_open(zip_path.c_str(), 0, &error);

    if (!archive) {
        return {};
    }

    zip_int64_t file_index = zip_name_locate(archive, file_name.c_str(), 0);
    if (file_index < 0) {
        zip_close(archive);
        return {};
    }

    zip_file_t* file = zip_fopen_index(archive, file_index, 0);
    if (!file) {
        zip_close(archive);
        return {};
    }

    std::vector<uint8_t> content;
    char buffer[1024];
    zip_int64_t bytes_read;
    while ((bytes_read = zip_fread(file, buffer, sizeof(buffer)))) {
        if (bytes_read < 0) {
            zip_fclose(file);
            zip_close(archive);
            return {};
        }
        content.insert(content.end(), buffer, buffer + bytes_read);
    }

    zip_fclose(file);
    zip_close(archive);

    return content;
}

auto unzip(const std::string& zip_path, const std::string& output_dir,
           std::function<void(uint64_t current, uint64_t total, const char* fileName)> progress) -> int {

    auto create_directory = [](const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                return true;
            }
        }
        createDirTree(path);
        if (stat(path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                return true;
            }
        }
        return false;
    };

    int error = 0;
    zip_t* archive = zip_open(zip_path.c_str(), 0, &error);

    if (!archive) {
        return RP_UP_EOF;
    }

    if (!create_directory(output_dir)) {
        zip_close(archive);
        return RP_UP_ECD;
    }

    zip_int64_t num_entries = zip_get_num_entries(archive, 0);

    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* file_name = zip_get_name(archive, i, 0);
        if (!file_name) {
            progress((uint64_t)i + 1, (uint64_t)num_entries, "");
            continue;
        }

        std::string full_path = output_dir + "/" + file_name;

        size_t last_slash = full_path.find_last_of('/');
        if (last_slash != std::string::npos) {
            std::string dir_path = full_path.substr(0, last_slash);
            if (!create_directory(dir_path)) {
                removeDirectory(output_dir);
                zip_close(archive);
                return RP_UP_EUF;
            }
        }

        struct stat st;
        if (stat(full_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                progress((uint64_t)i + 1, (uint64_t)num_entries, file_name);
                continue;
            }
        }

        zip_file_t* file = zip_fopen_index(archive, i, 0);
        if (!file) {
            removeDirectory(output_dir);
            zip_close(archive);
            return RP_UP_EUF;
        }

        std::ofstream out_file(full_path, std::ios::binary);
        if (!out_file) {
            zip_fclose(file);
            removeDirectory(output_dir);
            zip_close(archive);
            return RP_UP_EUF;
        }

        char buffer[1024];
        zip_int64_t bytes_read;
        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            out_file.write(buffer, bytes_read);
        }
        progress((uint64_t)i + 1, (uint64_t)num_entries, file_name);
        out_file.close();
        zip_fclose(file);
    }

    zip_close(archive);
    return RP_UP_OK;
}