
#include <dirent.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "common/rp_arb.h"
#include "common/version.h"
#include "main.h"
#include "rp.h"
#include "rp_hw-profiles.h"
#include "web/rp_client.h"

enum req_status_e {
    NONE = 0,
    REQ_UPDATE = 1,
    FILE_ERR = 2,
    FILE_ERR_TO_LONG = 3,
    FILE_ERR_PARS_ERR = 4,
    FILE_ERR_CANT_RENAME = 5,
    FILE_RENAME_DONE = 6,
    FILE_ERR_CANT_CHANGE_COLOR = 7,
    FILE_CHANGE_COLOR_DONE = 8,
    FILE_ERR_PARS_COU_ERR = 9,
};

CStringParameter req_check_file("RP_REQ_CHECK_FILE", CBaseParameter::RW, "", 0);
CStringParameter req_check_file_coe("RP_REQ_CHECK_FILE_COE", CBaseParameter::RW, "", 0);
CIntParameter req_status("RP_REQ_STATUS", CBaseParameter::RW, 0, 0, 0, 100);
CStringParameter req_files_list("RP_FILES_LIST", CBaseParameter::RW, "", 0);
CStringParameter req_rename_file("RP_RENAME_FILE", CBaseParameter::RW, "", 0);
CStringParameter req_change_color("RP_CHANGE_COLOR", CBaseParameter::RW, "", 0);
CFloatParameter max_gain("MAX_GAIN", CBaseParameter::RW, 1, 0, 0, 100);

auto getDACChannels() -> uint8_t {
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC channels count");
    }
    return c;
}

bool getOutFullScale(float* f) {
    auto count = getDACChannels();
    float z = -1;
    for (int i = 0; i < count; i++) {
        float g;
        if (rp_HPGetFastDACOutFullScale(i, &g) != RP_HP_OK)
            return false;
        if (z == -1) {
            z = g;
        } else if (z != g) {
            return false;
        }
    }
    *f = z;
    return true;
}

const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya arb manager application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading arb manager %s-%s.\n", VERSION_STR, REVISION_STR);
    float gain;
    if (getOutFullScale(&gain)) {
        max_gain.Value() = gain;
    }
    rp_ARBInit();
    rp_WC_Init();

    CDataManager::GetInstance()->SetParamInterval(50);
    CDataManager::GetInstance()->SetSignalInterval(50);
    sendFilesInfo();
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading arb manager %s-%s.\n", VERSION_STR, REVISION_STR);
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

auto decimateSignal(float* data, uint32_t size, float* out, uint32_t* sizeOut) -> void {
    int newsize = 1024;
    float rate = (float)newsize / (float)size;
    if (rate < 1) {
        *sizeOut = 0;
        int prev_x = -1;
        for (uint32_t i = 0; i < size; i++) {
            int x = (float)i * (float)rate;
            out[x] = data[i];
            if (prev_x != x) {
                (*sizeOut)++;
                prev_x = x;
            }
        }
    } else {
        for (uint32_t i = 0; i < size; i++) {
            out[i] = data[i];
        }
        *sizeOut = size;
    }
}

void sendFilesInfo() {
    uint32_t count = 0;
    rp_ARBLoadFiles();
    rp_ARBGetCount(&count);
    std::string req;
    for (uint32_t i = 0; i < count; i++) {
        std::string fileName;
        std::string name;
        uint32_t dataSize;
        float data[DAC_BUFFER_SIZE];
        uint32_t dataSizeOut;
        float dataOut[DAC_BUFFER_SIZE];
        bool is_valid;
        uint32_t color;

        if (rp_ARBGetFileName(i, &fileName) != RP_ARB_FILE_OK)
            continue;
        if (rp_ARBGetName(i, &name) != RP_ARB_FILE_OK)
            continue;
        if (rp_ARBGetSignal(i, data, &dataSize) != RP_ARB_FILE_OK)
            continue;
        if (rp_ARBIsValid(name, &is_valid) != RP_ARB_FILE_OK)
            continue;
        if (rp_ARBGetColor(i, &color) != RP_ARB_FILE_OK)
            continue;

        decimateSignal(data, dataSize, dataOut, &dataSizeOut);
        std::string sig = std::to_string(dataSizeOut) + "\t";
        for (uint32_t j = 0; j < dataSizeOut; j++) {
            if (j != 0) {
                sig += ";";
            }
            sig += std::to_string(dataOut[j]);
        }

        req += fileName + "\t" + name + "\t" + std::to_string(is_valid) + "\t" + std::to_string(color) + "\t" + sig + "\n";
    }
    req_files_list.Value() = req;
}

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

void UpdateParams(void) {
    req_check_file.Update();
    req_check_file_coe.Update();
    req_files_list.Update();
}

void PostUpdateSignals(void) {}

void UpdateSignals(void) {}

void OnNewParams(void) {
    if (req_check_file.IsNewValue()) {
        req_check_file.Update();
        TRACE_SHORT("Request create file")
        int res = rp_ARBGenFileCSV(req_check_file.Value());
        req_check_file.Value() = "";
        switch (res) {
            case RP_ARB_FILE_ERR:
                req_status.SendValue(FILE_ERR);
                break;
            case RP_ARB_FILE_TO_LONG:
                req_status.SendValue(FILE_ERR_TO_LONG);
                break;
            case RP_ARB_FILE_PARSE_ERR:
                req_status.SendValue(FILE_ERR_PARS_ERR);
                break;
            default:
                req_status.SendValue(REQ_UPDATE);
                break;
        }
    }

    if (req_check_file_coe.IsNewValue()) {
        req_check_file_coe.Update();
        TRACE_SHORT("Request create coe file")
        int res = rp_ARBGenFileCOE(req_check_file_coe.Value());
        req_check_file_coe.Value() = "";
        switch (res) {
            case RP_ARB_FILE_ERR:
                req_status.SendValue(FILE_ERR);
                break;
            case RP_ARB_FILE_TO_LONG:
                req_status.SendValue(FILE_ERR_TO_LONG);
                break;
            case RP_ARB_FILE_PARSE_ERR:
                req_status.SendValue(FILE_ERR_PARS_COU_ERR);
                break;
            default:
                req_status.SendValue(REQ_UPDATE);
                break;
        }
    }

    if (req_files_list.IsNewValue()) {
        req_files_list.Update();
        if (req_files_list.Value() != "") {
            TRACE_SHORT("Request files list")
            req_files_list.Value() = "";
            sendFilesInfo();
        }
    }

    if (req_rename_file.IsNewValue()) {
        req_rename_file.Update();
        auto str = req_rename_file.Value();
        req_rename_file.Value() = "";
        auto items = split(str, "\n");
        auto id = items[0];
        auto new_name = items[1];
        uint32_t count = 0;
        if (rp_ARBGetCount(&count) == RP_ARB_FILE_OK) {
            for (uint32_t i = 0; i < count; i++) {
                std::string fname = "";
                if (rp_ARBGetFileName(i, &fname) == RP_ARB_FILE_OK) {
                    if (fname == id) {
                        TRACE_SHORT("%s %s\n", fname.c_str(), id.c_str());
                        auto res = rp_ARBRenameFile(i, new_name);
                        if (res == RP_ARB_FILE_CANT_RENAME) {
                            req_status.SendValue(FILE_ERR_CANT_RENAME);
                            break;
                        } else {
                            req_status.SendValue(FILE_RENAME_DONE);
                            break;
                        }
                    }
                }
            }
        }
    }

    if (req_change_color.IsNewValue()) {
        req_change_color.Update();
        auto str = req_change_color.Value();
        req_change_color.Value() = "";
        auto items = split(str, "\n");
        auto id = items[0];
        auto color = std::stoul(items[1], nullptr, 0);

        uint32_t count = 0;
        if (rp_ARBGetCount(&count) == RP_ARB_FILE_OK) {
            for (uint32_t i = 0; i < count; i++) {
                std::string fname = "";
                if (rp_ARBGetFileName(i, &fname) == RP_ARB_FILE_OK) {
                    if (fname == id) {
                        TRACE_SHORT("%s %s\n", fname.c_str(), id.c_str());
                        auto res = rp_ARBSetColor(i, color);
                        if (res == RP_ARB_FILE_ERR) {
                            req_status.SendValue(FILE_ERR_CANT_CHANGE_COLOR);
                            break;
                        } else {
                            req_status.SendValue(FILE_CHANGE_COLOR_DONE);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void OnNewSignals(void) {}
