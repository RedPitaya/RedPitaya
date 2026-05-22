/**
 * $Id: rp_eeprom.c 996 2014-02-04 09:36:58Z ales.bardorfer $
 *
 * @brief Red Pitaya calibration EEPROM library routines.
 *
 * @Author Crt Valentincic <crt.valentincic@redpitaya.com>
 *         Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>

#include "common/rp_updater_common.h"
#include "rp_eeprom.h"

const char* c_wpCalParDesc_v1[eCalParEnd_v1][20] = {{"FE_CH1_FS_G_HI"},
                                                    {"FE_CH2_FS_G_HI"},
                                                    {"FE_CH1_FS_G_LO"},
                                                    {"FE_CH2_FS_G_LO"},
                                                    {"FE_CH1_DC_offs"},
                                                    {"FE_CH2_DC_offs"},
                                                    {"BE_CH1_FS"},
                                                    {"BE_CH2_FS"},
                                                    {"BE_CH1_DC_offs"},
                                                    {"BE_CH2_DC_offs"},
                                                    {"Magic"},
                                                    {"FE_CH1_DC_offs_HI"},
                                                    {"FE_CH2_DC_offs_HI"},
                                                    {"LOW_FILTER_AA_CH1"},
                                                    {"LOW_FILTER_BB_CH1"},
                                                    {"LOW_FILTER_PP_CH1"},
                                                    {"LOW_FILTER_KK_CH1"},
                                                    {"LOW_FILTER_AA_CH2"},
                                                    {"LOW_FILTER_BB_CH2"},
                                                    {"LOW_FILTER_PP_CH2"},
                                                    {"LOW_FILTER_KK_CH2"},
                                                    {"HI_FILTER_AA_CH1"},
                                                    {"HI_FILTER_BB_CH1"},
                                                    {"HI_FILTER_PP_CH1"},
                                                    {"HI_FILTER_KK_CH1"},
                                                    {"HI_FILTER_AA_CH2"},
                                                    {"HI_FILTER_BB_CH2"},
                                                    {"HI_FILTER_PP_CH2"},
                                                    {"HI_FILTER_KK_CH2"}};

const char* c_wpCalParDesc_v2[eCalParEnd_v2][20] = {{"OSC_CH1_HIGH"},        {"OSC_CH2_HIGH"},        {"OSC_CH3_HIGH"},       {"OSC_CH4_HIGH"},        {"OSC_CH1_LOW"},
                                                    {"OSC_CH2_LOW"},         {"OSC_CH3_LOW"},         {"OSC_CH4_LOW"},        {"OSC_CH1_HIGH_OFFSET"}, {"OSC_CH1_HIGH_OFFSET"},
                                                    {"OSC_CH1_HIGH_OFFSET"}, {"OSC_CH1_HIGH_OFFSET"}, {"OSC_CH1_LOW_OFFSET"}, {"OSC_CH1_LOW_OFFSET"},  {"OSC_CH1_LOW_OFFSET"},
                                                    {"OSC_CH1_LOW_OFFSET"},

                                                    {"OSC_CH1_HIGH_AA"},     {"OSC_CH1_HIGH_BB"},     {"OSC_CH1_HIGH_PP"},    {"OSC_CH1_HIGH_KK"},     {"OSC_CH1_LOW_AA"},
                                                    {"OSC_CH1_LOW_BB"},      {"OSC_CH1_LOW_PP"},      {"OSC_CH1_LOW_KK"},

                                                    {"OSC_CH2_HIGH_AA"},     {"OSC_CH2_HIGH_BB"},     {"OSC_CH2_HIGH_PP"},    {"OSC_CH2_HIGH_KK"},     {"OSC_CH2_LOW_AA"},
                                                    {"OSC_CH2_LOW_BB"},      {"OSC_CH2_LOW_PP"},      {"OSC_CH2_LOW_KK"},

                                                    {"OSC_CH3_HIGH_AA"},     {"OSC_CH3_HIGH_BB"},     {"OSC_CH3_HIGH_PP"},    {"OSC_CH3_HIGH_KK"},     {"OSC_CH3_LOW_AA"},
                                                    {"OSC_CH3_LOW_BB"},      {"OSC_CH3_LOW_PP"},      {"OSC_CH3_LOW_KK"},

                                                    {"OSC_CH4_HIGH_AA"},     {"OSC_CH4_HIGH_BB"},     {"OSC_CH4_HIGH_PP"},    {"OSC_CH4_HIGH_KK"},     {"OSC_CH4_LOW_AA"},
                                                    {"OSC_CH4_LOW_BB"},      {"OSC_CH4_LOW_PP"},      {"OSC_CH4_LOW_KK"}

};

const char* c_wpCalParDesc_v3[eCalParEnd_v3][20] = {
    {"GEN_CH1_G_1"},     {"GEN_CH2_G_1"},     {"GEN_CH1_OFF_1"},     {"GEN_CH2_OFF_1"},     {"GEN_CH1_G_5"},     {"GEN_CH2_G_5"},     {"GEN_CH1_OFF_5"},     {"GEN_CH2_OFF_5"},
    {"OSC_CH1_G_1_AC"},  {"OSC_CH2_G_1_AC"},  {"OSC_CH1_OFF_1_AC"},  {"OSC_CH2_OFF_1_AC"},  {"OSC_CH1_G_1_DC"},  {"OSC_CH2_G_1_DC"},  {"OSC_CH1_OFF_1_DC"},  {"OSC_CH2_OFF_1_DC"},
    {"OSC_CH1_G_20_AC"}, {"OSC_CH2_G_20_AC"}, {"OSC_CH1_OFF_20_AC"}, {"OSC_CH2_OFF_20_AC"}, {"OSC_CH1_G_20_DC"}, {"OSC_CH2_G_20_DC"}, {"OSC_CH1_OFF_20_DC"}, {"OSC_CH2_OFF_20_DC"}};

int getCalibSize(rp_HPeModels_t model) {
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_BO_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_125_14_Z7020_TI_v1_3:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
            return eCalParEnd_v1;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return eCalPar_F_LOW_AA_CH1;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            return eCalParEnd_v2;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return eCalParEnd_v3;
        default: {
            fprintf(stderr, "[Error:getCalibSize] Unknown model: %d.\n", model);
            return -1;
        }
    }
}

void RpPrintEepromCalData(rp_HPeModels_t model, rp_eepromWpData_t* _eepromData, bool verb, bool hex) {
    int size = 0;

    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_BO_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_125_14_Z7020_TI_v1_3:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1: {
            size = eCalParEnd_v1;
            if (_eepromData->feCalPar[eCalParMagic] == (int32_t)CALIB_MAGIC) {
                size = eCalPar_F_LOW_AA_CH1;
            }
            if (verb) {
                printf(hex ? "dataStructureId = 0x%X\n" : "dataStructureId = %d\n", _eepromData->dataStructureId);
                printf(hex ? "wpCheck = 0x%X\n" : "wpCheck = %d\n", _eepromData->wpCheck);
                for (int i = 0; i < size; ++i) {
                    printf(hex ? "%s = 0x%X\n" : "%s = %d\n", c_wpCalParDesc_v1[i][0], _eepromData->feCalPar[i]);
                }
                return;
            }
            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
        case STEM_125_14_Z7020_4IN_BO_v1_3: {
            size = eCalParEnd_v2;
            if (verb) {
                printf(hex ? "dataStructureId = 0x%X\n" : "dataStructureId = %d\n", _eepromData->dataStructureId);
                printf(hex ? "wpCheck = 0x%X\n" : "wpCheck = %d\n", _eepromData->wpCheck);
                for (int i = 0; i < size; ++i) {
                    printf(hex ? "%s = 0x%X\n" : "%s = %d\n", c_wpCalParDesc_v2[i][0], _eepromData->feCalPar[i]);
                }
                return;
            }
            break;
        }
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120: {
            size = eCalParEnd_v3;
            if (verb) {
                printf(hex ? "dataStructureId = 0x%X\n" : "dataStructureId = %d\n", _eepromData->dataStructureId);
                printf(hex ? "wpCheck = 0x%X\n" : "wpCheck = %d\n", _eepromData->wpCheck);
                for (int i = 0; i < size; ++i) {
                    printf(hex ? "%s = 0x%X\n" : "%s = %d\n", c_wpCalParDesc_v3[i][0], _eepromData->feCalPar[i]);
                }
                return;
            }
            break;
        }

        default: {
            fprintf(stderr, "[Error:calib_LoadFromFactoryZone] Unknown model: %d.\n", model);
            return;
        }
    }

    for (int i = 0; i < size; ++i) {
        fprintf(stdout, hex ? "0x%X\t" : "%20d", _eepromData->feCalPar[i]);
    }
    fprintf(stdout, "\n");
}

void RpPrintEepromCalDataUni(rp_eepromUniData_t* _eepromData, bool verb, bool hex) {
    if (verb) {
        printf(hex ? "dataStructureId = 0x%X\n" : "dataStructureId = %d\n", _eepromData->dataStructureId);
        printf(hex ? "wpCheck = 0x%X\n" : "wpCheck = %d\n", _eepromData->wpCheck);
        printf(hex ? "count = 0x%X\n" : "count = %d\n", _eepromData->count);
        for (int i = 0; i < _eepromData->count; ++i) {
            std::string name;
            rp_GetNameOfUniversalId(_eepromData->item[i].id, &name);
            printf(hex ? "%s (%d) = 0x%X\n" : "%s (%d) = %d\n", name.c_str(), _eepromData->item[i].id, _eepromData->item[i].value);
        }
        return;
    }
    for (int i = 0; i < _eepromData->count; ++i) {
        fprintf(stdout, hex ? "0x%X\t0x%X\t" : "%20d%20d", _eepromData->item[i].id, _eepromData->item[i].value);
    }
    fprintf(stdout, "\n");
}

void print_eeprom(rp_HPeModels_t model, rp_eepromWpData_t* data, int mode) {
    /* Print */
    if (mode & WANT_VERBOSE) {
        RpPrintEepromCalData(model, data, true, mode & WANT_HEX);
    } else {
        if (!(mode & WANT_Z_MODE)) {
            RpPrintEepromCalData(model, data, false, mode & WANT_HEX);
        } else {
            switch (model) {
                case STEM_125_10_v1_0:
                case STEM_125_14_v1_0:
                case STEM_125_14_v1_1:
                case STEM_125_14_LN_v1_1:
                case STEM_125_14_LN_BO_v1_1:
                case STEM_125_14_LN_CE1_v1_1:
                case STEM_125_14_LN_CE2_v1_1:
                case STEM_125_14_Z7020_v1_0:
                case STEM_125_14_Z7020_LN_v1_1:
                case STEM_125_14_v2_0:
                case STEM_125_14_BO_v2_0:
                case STEM_125_14_Pro_v2_0:
                case STEM_125_14_Pro_BO_v2_0:
                case STEM_125_14_Z7020_Pro_v1_0:
                case STEM_125_14_Z7020_Pro_v2_0:
                case STEM_125_14_Z7020_Pro_BO_v2_0:
                case STEM_125_14_Z7020_Ind_v2_0:
                case STEM_125_14_Z7020_LL_v1_1:
                case STEM_125_14_Z7020_LL_v1_2:
                case STEM_125_14_Z7020_TI_v1_3:
                case STEM_65_16_Z7020_LL_v1_1:
                case STEM_65_16_Z7020_TI_v1_3:
                case STEM_122_16SDR_v1_0:
                case STEM_122_16SDR_v1_1: {
                    fprintf(stdout,
                            "%20d %20d %20d %20d %20d %20d %20d %20d %20d %20d %20d %20d\n",
                            data->feCalPar[eCalPar_FE_CH1_DC_offs],
                            data->feCalPar[eCalPar_FE_CH2_DC_offs],
                            data->feCalPar[eCalPar_FE_CH1_FS_G_LO],
                            data->feCalPar[eCalPar_FE_CH2_FS_G_LO],
                            data->feCalPar[eCalPar_FE_CH1_DC_offs_HI],
                            data->feCalPar[eCalPar_FE_CH2_DC_offs_HI],
                            data->feCalPar[eCalPar_FE_CH1_FS_G_HI],
                            data->feCalPar[eCalPar_FE_CH2_FS_G_HI],
                            data->feCalPar[eCalPar_BE_CH1_DC_offs],
                            data->feCalPar[eCalPar_BE_CH2_DC_offs],
                            data->feCalPar[eCalPar_BE_CH1_FS],
                            data->feCalPar[eCalPar_BE_CH2_FS]);
                    break;
                }

                case STEM_125_14_Z7020_4IN_v1_0:
                case STEM_125_14_Z7020_4IN_v1_2:
                case STEM_125_14_Z7020_4IN_v1_3:
                case STEM_125_14_Z7020_4IN_BO_v1_3:
                case STEM_250_12_v1_0:
                case STEM_250_12_v1_1:
                case STEM_250_12_v1_2:
                case STEM_250_12_v1_2a:
                case STEM_250_12_v1_2b:
                case STEM_250_12_120: {
                    fprintf(stdout, "Unsupport mode\n");
                    break;
                }

                default: {
                    fprintf(stderr, "[Error:print_eeprom] Unknown model: %d.\n", model);
                    break;
                }
            }
        }
    }
}

void print_eepromUni(rp_eepromUniData_t* data, int mode) {
    RpPrintEepromCalDataUni(data, mode & WANT_VERBOSE, mode & WANT_HEX);
}

// Helper function to read stdin efficiently
bool readStdinToVector(std::vector<char>& out_data) {
    constexpr size_t BUFFER_SIZE = 65536;
    std::vector<char> buffer(BUFFER_SIZE);
    out_data.clear();

    while (std::cin) {
        std::cin.read(buffer.data(), BUFFER_SIZE);
        const size_t bytes_read = std::cin.gcount();
        if (bytes_read > 0) {
            out_data.insert(out_data.end(), buffer.data(), buffer.data() + bytes_read);
        }
        if (std::cin.eof())
            break;
        if (std::cin.fail() && !std::cin.eof())
            return false;
    }

    return true;
}

// Helper function to get board MAC address
bool getBoardMAC(uint8_t* mac) {
    char* mac_str = nullptr;
    if (rp_HPGetModelETH_MAC_Address(&mac_str) != RP_OK || mac_str == nullptr) {
        fprintf(stderr, "ERROR: Failed to get board MAC address!\n");
        return false;
    }

    // Parse MAC string "XX:XX:XX:XX:XX:XX" to bytes
    unsigned int values[6];
    if (sscanf(mac_str, "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) != 6) {
        fprintf(stderr, "ERROR: Failed to parse MAC address!\n");
        return false;
    }

    for (int i = 0; i < 6; i++) {
        mac[i] = static_cast<uint8_t>(values[i]);
    }

    return true;
}

std::string formatMAC(const uint8_t* mac) {
    char buffer[18];
    snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buffer);
}

// Helper function to get board model name
bool getBoardModelName(std::string& model_name) {
    char* name = nullptr;
    if (rp_HPGetModelName(&name) != RP_OK || name == nullptr) {
        fprintf(stderr, "ERROR: Failed to get board model name!\n");
        return false;
    }
    model_name = name;
    return true;
}

bool getBoardModel(rp_HPeModels_t& model) {
    if (rp_HPGetModel(&model) != RP_OK) {
        fprintf(stderr, "ERROR: Failed to get board model!\n");
        return false;
    }
    return true;
}

std::string bytesToAsciiString(const std::vector<char>& data, size_t max_bytes) {
    size_t bytes_to_process = std::min(data.size(), max_bytes);
    return std::string(data.begin(), data.begin() + bytes_to_process);
}

std::vector<std::string> split(const std::string& text, const std::vector<char>& delimiters) {
    std::vector<std::string> result;
    std::string current_token;
    for (char c : text) {
        if (std::find(delimiters.begin(), delimiters.end(), c) != delimiters.end()) {
            if (!current_token.empty()) {
                result.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }
    if (!current_token.empty()) {
        result.push_back(current_token);
    }
    return result;
}

void printBackupInfo(const char* filename, uint32_t want_bits) {
    auto formatTimestamp = [](uint64_t timestamp) -> std::string {
        char time_str[64];
        time_t backup_time = static_cast<time_t>(timestamp);
        struct tm* tm_info = localtime(&backup_time);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        return std::string(time_str);
    };

    // Read binary file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        fprintf(stderr, "ERROR: Failed to open file '%s'!\n", filename);
        exit(EXIT_FAILURE);
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size == 0) {
        fprintf(stderr, "ERROR: File is empty!\n");
        exit(EXIT_FAILURE);
    }

    // Read entire file
    std::vector<char> all_data(file_size);
    if (!file.read(all_data.data(), file_size)) {
        fprintf(stderr, "ERROR: Failed to read file!\n");
        exit(EXIT_FAILURE);
    }
    file.close();

    constexpr size_t MD5_SIZE = 32;
    constexpr size_t HEADER_SIZE = sizeof(BackupHeader);
    constexpr size_t FULL_HEADER_SIZE = MD5_SIZE + HEADER_SIZE;

    if (all_data.size() <= FULL_HEADER_SIZE) {
        fprintf(stderr, "ERROR: Invalid backup data!\n");
        exit(EXIT_FAILURE);
    }

    // Extract MD5 hash
    std::string file_hash = bytesToAsciiString(all_data, MD5_SIZE);

    // Extract backup header
    BackupHeader header;
    memcpy(&header, all_data.data() + MD5_SIZE, HEADER_SIZE);

    // Verify MD5
    std::string calculated_hash;
    const auto data_span = std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(all_data.data() + MD5_SIZE), all_data.size() - MD5_SIZE);

    bool hash_valid = false;
    if (rp_UpdaterGetMD5(std::vector<uint8_t>(data_span.begin(), data_span.end()), &calculated_hash) == RP_UP_OK) {
        hash_valid = (file_hash == calculated_hash);
    }

    // Print backup information
    printf("=== Backup Information ===\n");
    printf("File:          %s\n", filename);
    printf("File size:     %zu bytes\n", all_data.size());
    printf("MD5 hash:      %s\n", file_hash.c_str());
    printf("Hash valid:    %s\n", hash_valid ? "YES" : "NO");
    printf("\n");
    printf("Board model:   %s (%d)\n", header.model_name, static_cast<int>(header.model_id));
    printf("MAC address:   %s\n", formatMAC(header.mac).c_str());
    printf("Created:       %s\n", formatTimestamp(header.timestamp).c_str());
    printf("Calib. size:   %zu bytes\n", all_data.size() - FULL_HEADER_SIZE);
    printf("\n");

    // Print EEPROM data based on version
    const uint8_t* calib_data = reinterpret_cast<const uint8_t*>(all_data.data() + FULL_HEADER_SIZE);
    size_t calib_size = all_data.size() - FULL_HEADER_SIZE;
    if (want_bits & WANT_VERBOSE) {
        if (calib_size > 0 && calib_data[0] < RP_HW_PACK_ID_V5) {
            print_eeprom(header.model_id, (rp_eepromWpData_t*)calib_data, want_bits);
        } else if (calib_size > 0) {
            print_eepromUni((rp_eepromUniData_t*)calib_data, want_bits);
        }
    }
    if (want_bits & WANT_PRINT) {
        rp_calib_params_t calib;
        auto ret = rp_CalibConvertEEPROM(const_cast<uint8_t*>(calib_data), calib_size, &calib);
        if (ret) {
            fprintf(stderr, "ERROR: Convert data failed!\n");
            exit(EXIT_FAILURE);
        }
        rp_CalibPrint(&calib);
    }

    exit(EXIT_SUCCESS);
}