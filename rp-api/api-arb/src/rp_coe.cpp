/**
 * $Id: $
 *
 * @brief Red Pitaya library arb api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include "rp_coe.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include "rp_arb.h"
#include "rp_hw-profiles.h"

uint8_t bits = rp_HPGetFastDACBitsOrDefault();
float fs = rp_HPGetHWDACFullScaleOrDefault();

Radix rp_ARBParseRadix(const string& line) {
    string radixStr = line.substr(line.find('=') + 1);
    radixStr.erase(remove(radixStr.begin(), radixStr.end(), ';'), radixStr.end());
    radixStr.erase(remove(radixStr.begin(), radixStr.end(), ' '), radixStr.end());

    if (radixStr == "16")
        return HEX;
    if (radixStr == "10")
        return DEC;
    if (radixStr == "2")
        return BIN;
    return UNKNOWN;
}

uint32_t rp_ARBParseValue(const string& token, Radix radix, double* _value) {
    try {
        size_t pos = 0;
        uint32_t value;

        switch (radix) {
            case HEX:
                value = stoul(token, &pos, 16);
                break;
            case DEC:
                value = stoul(token, &pos, 10);
                break;
            case BIN:
                value = stoul(token, &pos, 2);
                break;
            default: {
                ERROR_LOG("Unknown radix")
                return RP_ARB_FILE_PARSE_ERR;
            }
        }

        if (pos != token.size()) {
            ERROR_LOG("Invalid characters in number")
            return RP_ARB_FILE_PARSE_ERR;
        }

        uint32_t maxValue = 1 << (bits);
        if (value > maxValue) {
            ERROR_LOG("Value exceeds %d-bit range", bits)
            return RP_ARB_FILE_PARSE_ERR;
        }

        uint32_t signValue = 1 << (bits - 1);
        if (signValue & value) {
            uint32_t neg_mask = 0xFFFFFFFF ^ (signValue - 1);
            value = value | neg_mask;
        }
        int32_t valueI = (int32_t)value;
        *_value = (double)valueI / (double)(signValue - 1) * fs;
        return RP_ARB_FILE_OK;
    } catch (const exception& e) {
        ERROR_LOG("Error parsing value '%s': %s", token.c_str(), e.what())
        return RP_ARB_FILE_PARSE_ERR;
    }
}

int rp_ARBParseCoeFile(const string& filename, CoeData* data) {
    ifstream file(filename);
    if (!file.is_open()) {
        return RP_ARB_FILE_ERR;
    }

    // CoeData result;
    (*data).radix = UNKNOWN;
    bool dataSection = false;
    string line;

    while (getline(file, line)) {
        size_t commentPos = line.find(';');
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }

        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty())
            continue;

        if (line.find("memory_initialization_radix=") != string::npos) {
            (*data).radix = rp_ARBParseRadix(line);
            if ((*data).radix == UNKNOWN) {
                ERROR_LOG("Unknown radix specified in file")
                return RP_ARB_FILE_PARSE_ERR;
            }
            continue;
        }

        if (line.find("memory_initialization_vector=") != string::npos) {
            dataSection = true;
            line = line.substr(line.find('=') + 1);
        }

        if (dataSection) {
            if (line.back() == ';') {
                line.pop_back();
                dataSection = false;
            }

            size_t pos = 0;
            while ((pos = line.find(',')) != string::npos) {
                string token = line.substr(0, pos);
                if (!token.empty()) {
                    double value;
                    auto ret = rp_ARBParseValue(token, (*data).radix, &value);
                    if (ret == RP_ARB_FILE_OK) {
                        (*data).data.push_back(value);
                    } else {
                        return ret;
                    }
                }
                line.erase(0, pos + 1);
            }

            if (!line.empty()) {
                double value;
                auto ret = rp_ARBParseValue(line, (*data).radix, &value);
                if (ret == RP_ARB_FILE_OK) {
                    (*data).data.push_back(value);
                } else {
                    return ret;
                }
            }
        }
    }

    if ((*data).radix == UNKNOWN) {
        ERROR_LOG("Radix not specified in COE file")
        return RP_ARB_FILE_PARSE_ERR;
    }

    if ((*data).data.empty()) {
        ERROR_LOG("No data found in COE file")
        return RP_ARB_FILE_PARSE_ERR;
    }

    return RP_ARB_FILE_OK;
}