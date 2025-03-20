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

#include "rp_updater_common.h"
#include <openssl/evp.h>
#include <openssl/md5.h>
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