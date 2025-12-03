/**
 * $Id: $
 *
 * @brief Red Pitaya library updater api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include "rp_updater_common.h"
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

int rp_UpdaterGetMD5(std::string fileName, std::string* hash) {
    std::ifstream file(fileName, std::ios::binary);
    if (!file) {
        return RP_UP_EOF;
    }

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_md5();

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLen;

    EVP_DigestInit_ex(mdctx, md, nullptr);

    char buffer[1024];
    while (file.good()) {
        file.read(buffer, sizeof(buffer));
        EVP_DigestUpdate(mdctx, buffer, file.gcount());
    }

    EVP_DigestFinal_ex(mdctx, digest, &digestLen);
    EVP_MD_CTX_free(mdctx);

    std::ostringstream hexStream;
    for (unsigned int i = 0; i < digestLen; ++i) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }

    *hash = hexStream.str();
    return RP_UP_OK;
}

int rp_UpdaterGetMD5(const std::vector<uint8_t>& data, std::string* hash) {
    std::vector<uint8_t> md5_hash(MD5_DIGEST_LENGTH);

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);

    if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        return RP_UP_ECM;
    }

    unsigned int md5_len = MD5_DIGEST_LENGTH;
    EVP_DigestFinal_ex(ctx, md5_hash.data(), &md5_len);
    EVP_MD_CTX_free(ctx);
    std::ostringstream hexStream;
    for (unsigned int i = 0; i < md5_len; ++i) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)md5_hash[i];
    }

    *hash = hexStream.str();
    return RP_UP_OK;
}

int rp_sortEcosystemFiles(std::vector<std::string>& files) {
    auto compareVersionStrings = [](const std::string& a, const std::string& b) {
        size_t start1 = a.find('-');
        size_t start2 = b.find('-');

        if (start1 == std::string::npos || start2 == std::string::npos) {
            return a < b;
        }

        size_t end1 = a.find('-', start1 + 1);
        size_t end2 = b.find('-', start2 + 1);

        std::string num1Str = a.substr(start1 + 1, end1 - (start1 + 1));
        std::string num2Str = b.substr(start2 + 1, end2 - (start2 + 1));

        double num1 = stod(num1Str);
        double num2 = stod(num2Str);

        if (num1 != num2) {
            return num1 < num2;
        }

        if (end1 == std::string::npos || end2 == std::string::npos) {
            return a < b;
        }

        size_t end3 = a.find('-', end1 + 1);
        size_t end4 = b.find('-', end2 + 1);

        std::string num3Str = a.substr(end1 + 1, end3 - (end1 + 1));
        std::string num4Str = b.substr(end2 + 1, end4 - (end2 + 1));

        int num3 = stoi(num3Str);
        int num4 = stoi(num4Str);

        return num3 < num4;
    };

    sort(files.begin(), files.end(), compareVersionStrings);

    return 0;
}
